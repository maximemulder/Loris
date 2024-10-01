<?php

namespace LORIS\redcap\Notifications;

use \LORIS\LorisInstance;
use \LORIS\redcap\RedcapHttpClient as HttpClient;
use \LORIS\redcap\Notifications\RedcapNotification as Notification;
use \LORIS\redcap\Models\IRedcapRecord;
use \LORIS\redcap\Models\RedcapRepeatedRecord;

class RedcapNotificationHandler
{
    private LorisInstance $loris;
    private HttpClient $client;
    private array $visit_mapping = [];

    public function __construct(LorisInstance $loris)
    {
        $this->loris  = $loris;
        $this->client = new HttpClient($this->loris);
    }

    public function handle(Notification $notification): void
    {
        $pscid           = $notification->getRecord();
        $visit_label     = $notification->getRedcapEventName();
        $instrument_name = $notification->getInstrument();


        // TEMP
        $fp = fopen('/data/tmp/redcap_request.log', 'w');
        fwrite($fp, json_encode([
            'PSCID'           => $pscid ,
            'Visit label'     => $visit_label,
            'Instrument name' => $instrument_name,
        ]));
        fclose($fp);
        die();

        // Acquire LOCK on table to make sure notifications are hanled only once.
        $this->_acquireNotificationLock($notification);

        try {
            // get data from redcap
            $records = $this->client->getRecords(
                $pscid,
                $visit_label,
                $instrument_name
            );
        } catch (\Throwable $e) {
            $this->_releaseNotificationLock();
            throw $e;
        }

        $this->_markNotificationAsHandled($notification);
        $this->_releaseNotificationLock();

        $updated = false;

        foreach ($records as $index => $record) {
            $instrument_name = $record->getInstrumentName();

            $instrument = $this->_instantiateInstrument(
                $pscid,
                $visit_label,
                $instrument_name
            );

            if (!$instrument->determineDataEntryAllowed()) {
                error_log(
                    "[redcap] instrument already 'complete': "
                    . $pscid . " "
                    . $visit_label . " "
                    . $instrument_name
                );
                continue;
            }

            $this->_assertDictionaryMatches(
                $instrument,
                $record
            );

            $this->_updateInstrument($instrument, $record);
            $updated = true;
        }

        if (!$updated) {
            $msg = sprintf(
                'Instrument not updatable. %s %s %s',
                $pscid,
                $visit_label,
                $instrument_name
            );
            throw new \LorisException($msg);
        }
        error_log("[redcap] instrument updated: "
            . $pscid . " "
            . $visit_label . " "
            . $instrument_name
        );
    }


    private function _assertDictionaryMatches(
        \NDB_BVL_Instrument $instrument,
        IRedcapRecord $record
    ): void {
        $instrument_name = $instrument->testName;

        $dict_names = [];
        // Remove instrument name from field names
        foreach ($instrument->getDataDictionary() as $field) {
            $field_name   = $field->getName();
            $dict_names[] = preg_replace("/^${instrument_name}_/", '', $field->getName());
        }

        // Remove LORIS insternal fields
        $dict_names = array_filter(
            $dict_names,
            function ($name) {
                return !in_array(
                    $name,
                    [
                        'Date_taken',
                        'Candidate_age',
                        'Window_Difference',
                        'Examiner',
                    ]
                );
            }
        );

        // Remove REDCap specific fields
        $redcap_form_name = $record->getFormName();
        $record_names = array_filter(
            $record->getPropertyNames(),
            function ($name) use ($redcap_form_name) {
                return !in_array(
                    $name,
                    [
                        'redcap_survey_identifier',
                        $redcap_form_name . '_timestamp',
                        $redcap_form_name . '_complete',
                    ]
                );
            }
        );

        // Remove instrument name from REDCap field names
        $record_names = array_map(
            function ($name) use ($redcap_form_name) {
                $retval = str_replace("{$redcap_form_name}", '', $name);
                $retval = ltrim($retval, '_');
                return $retval;
            },
            $record_names
        );

        $record_names = array_unique(
            array_reduce(
                $record_names,
                function ($names, $name) {
                    preg_match('/(.*)[_][_][_](.*)/', $name, $matches);
                    if (!empty($matches[1])) {
                        $name = $matches[1];
                    }
                    $names[] = $name;
                    return $names;
                },
                []
            )
        );

        // Make sure all REDCap fields exists in LORIS.
        // Note: Score fields should be removed to have an exact match.
        $missing_fields = array_diff($record_names, $dict_names);

        if (!empty($missing_fields)) {
            $msg = sprintf(
                "Missing fields in LORIS: %s",
                implode(', ', $missing_fields)
            );
            throw new \LorisException($msg);
        }
    }

    private function _instantiateInstrument(
        string $pscid,
        string $visit_label,
        string $instrument_name
    ): \NDB_BVL_Instrument {
        $commentid = $this->_getCommentID($pscid, $visit_label, $instrument_name);

        return \NDB_BVL_Instrument::factory(
            $this->loris,
            $instrument_name,
            $commentid,
            '',
            true
        );
    }

    private static function _formatEnumFields(array $assoc_values): array
    {
        $keys    = array_keys($assoc_values);
        $reduced = array_reduce(
            $keys,
            function ($carry, $item) use ($assoc_values) {
                $field_name = $item;
                $value      = $assoc_values[$field_name];

                preg_match('/(.*)[_][_][_](.*)/', $item, $matches);
                if (!empty($matches[1])) {
                    // It is an enum field
                    $new_fieldname = $matches[1];
                    $prev_value    = isset($carry[$new_fieldname]) ? [$carry[$new_fieldname]] : [];
                    $new_value     = $prev_value;

                    if ($value == '1') {
                        // The value is selected
                        $value     = [$matches[2]];
                        $new_value = [implode('{@}', array_merge($prev_value, $value))];
                    }

                    $value      = array_shift($new_value);
                    $field_name = $new_fieldname;
                }

                if (!isset($carry[$field_name])) {
                    $carry[$field_name] = null;
                }

                $carry[$field_name] = $value;

                return $carry;

            },
            []
        );

        return $reduced;
    }

    private function _updateInstrument(\NDB_BVL_Instrument $instrument, IRedcapRecord $record): void
    {
        $instrument_name   = $record->getFormName();
        $instrument_values = $record->toArray();
        $commentid         = $instrument->getCommentID();

        // Add Examiner
        $instrument_values['Examiner'] = $this->_getRedcapExaminerId();

        // Remove instrument name from field name
        foreach ($instrument_values as $key => $value) {
            $new_key = str_replace("{$instrument_name}", '', $key);
            $new_key = ltrim($new_key, '_');
            if ($key != $new_key) {
                $instrument_values[$new_key] = $value;
                unset($instrument_values[$key]);
            }
        }

        // -- Define/Add Date_taken
        // First, try based on 'dtt'
        // Then, if still null/undefined try based on 'timestamp_stop'
        // Then, if still null/undefined try based on 'timestamp_start'
        // Finally, if still null/undefined use current datetime
        $dt = \DateTime::createFromFormat('Y-m-d H:i:s', $instrument_values['dtt']);
        if (!$dt) {
            error_log("[redcap] Could not parse 'dtt': " . $instrument_values['dtt']);
        } else {
            $instrument_values['Date_taken'] = $dt->format('Y-m-d');
        }

        // if null/empty, try getting that based on the timestamp
        if (is_null($instrument_values['Date_taken']) || empty($instrument_values['Date_taken'])) {
            if (isset($instrument_values['timestamp'])) {
                $dt = \DateTime::createFromFormat('Y-m-d H:i:s', $instrument_values['timestamp']);
                if (!$dt) {
                    error_log("[redcap] Could not parse 'timestamp': " . $instrument_values['timestamp']);
                } else {
                    $instrument_values['Date_taken'] = $dt->format('Y-m-d');
                }
            }
        }
        // if null/empty, try getting that based on the timestamp_start
        if (is_null($instrument_values['Date_taken']) || empty($instrument_values['Date_taken'])) {
            if (isset($instrument_values['timestamp_start'])) {
                $dt = \DateTime::createFromFormat('Y-m-d H:i:s', $instrument_values['timestamp_start']);
                if (!$dt) {
                    error_log("[redcap] Could not parse 'timestamp_start': " . $instrument_values['timestamp_start']);
                } else {
                    $instrument_values['Date_taken'] = $dt->format('Y-m-d');
                }
            }
        }
        // if still null/empty, get the current date
        if (is_null($instrument_values['Date_taken']) || empty($instrument_values['Date_taken'])) {
            $instrument_values['Date_taken'] = (new \DateTimeImmutable())->format('Y-m-d');
        }

        // add the timestamp_stop in the values based on the last timestamp
        if (!is_null($instrument_values['timestamp'])
            && !empty($instrument_values['timestamp'])
        ) {
            // rename var to uniformize with other LORIS instruments
            // Duration will be calculated when _saveValues is called.
            $instrument_values['timestamp_stop'] = $instrument_values['timestamp'];
        }

        // locked timestamp only if data does not already exist
        $hasData = $this->loris->getDatabaseConnection()->pselectOne(
            "SELECT Data FROM flag WHERE commentId = :cid",
            ["cid" => $commentid]
        );
        if (is_null($hasData) || empty($hasData) || $hasData === '{}') {
            // add a "redcap value lock". We not want the timestamp to be edited
            // after we receive it from REDCap. This lock should be checked
            // before trying to edit in the related instrument.
            $instrument_values['timestamp_redcap_locked'] = 'locked';
        }

        // Aggregate enum values in a single field
        $instrument_values = $this->_formatEnumFields($instrument_values);

        $instrument->_saveValues($instrument_values);
        $instrument->score();
        $instrument->updateRequiredElementsCompletedFlag();

        // Mark data entry complete
        $stmt = $this->loris->getDatabaseConnection()->prepare(
            '
            UPDATE flag
            SET
              Data_entry = "Complete",
              Administration = "All",
              Validity = "Valid"
            WHERE
              CommentID = :commentid
        '
        );

        $stmt->execute(
            [
                'commentid' => $commentid
            ]
        );
    }

    private function _getCommentID(
        string $pscid,
        string $visit_label,
        string $instrument_name
    ): string {
        $visit_label = $this->_getLorisVisitLabelFromRedcapEventName($visit_label, $pscid);

        $db   = $this->loris->getDatabaseConnection();
        $rows = $db->pselect(
            '
            SELECT
              f.CommentID as CommentID
            FROM flag f
            LEFT JOIN session s
              ON (f.SessionID = s.ID)
            LEFT JOIN candidate c
              ON (c.CandID = s.CandID)
            WHERE
              c.PSCID = :pscid AND
              f.Test_name = :instrument AND
              s.Visit_label = :visit AND
              f.CommentID NOT LIKE "DDE_%"
            ',
            [
                'pscid'      => $pscid,
                'visit'      => $visit_label,
                'instrument' => $instrument_name
            ]
        );

        if (count($rows) > 1) {
            $msg = sprintf(
                'Multiple CommentID for pscid: %s, visit: %s, instrument: %s :: %s',
                $pscid,
                $visit_label,
                $instrument_name,
                json_encode($rows)
            );
            throw new \LorisException($msg);
        }

        if (empty($rows)) {
            $msg = sprintf(
                'No record in flag table for pscid: %s, visit: %s, instrument: %s',
                $pscid,
                $visit_label,
                $instrument_name
            );
            throw new \LorisException($msg);
        }

        $row = array_shift($rows);

        return $row['CommentID'];
    }

    private function _getRedcapExaminerId(): string
    {
        $db = $this->loris->getDatabaseConnection();
        return $db->pselectOne(
            '
            SELECT examinerID FROM examiners WHERE UPPER(full_name) = "REDCAP"
        ',
            []
        );
    }

    private function _getLorisVisitLabelFromRedcapEventName(string $name, string $pscid): ?string
    {
        if (empty($this->visit_mapping)) {
            foreach ($this->client->fetchEvents($pscid) as $event) {
                $redcap_name = trim($event['unique_event_name']);
                $loris_name  = trim($event['event_name']);
                $this->visit_mapping[$redcap_name] = $loris_name;
            }
        }

        return $this->visit_mapping[$name] ?? null;
    }

    private function _acquireNotificationLock(Notification $notification): void
    {
        $db = $this->loris->getDatabaseConnection();
        $db->run('LOCK TABLES redcap_notification as lock_1 WRITE');

        $query = '
            SELECT
              id
            FROM redcap_notification as lock_1
            WHERE
              received_dt = :v_received_dt AND
              project_id = :v_project_id AND
              record = :v_record AND
              redcap_event_name = :v_redcap_event_name AND
              instrument = :v_instrument AND
              handled_dt IS NULL
            FOR UPDATE
        ';

        $notification_params = [
            'v_received_dt'       => $notification->getReceivedDt()->format('Y-m-d H:i:s'),
            'v_project_id'        => $notification->getProjectId(),
            'v_record'            => $notification->getRecord(),
            'v_redcap_event_name' => $notification->getRedcapEventName(),
            'v_instrument'        => $notification->getInstrument()
        ];

        $stmt = $db->prepare($query);
        $stmt->execute($notification_params);
        $results = $stmt->fetchAll();

        if (empty($results)) {
            $this->_releaseNotificationLock();
            throw new \LorisException(
                'Notification already handled or not found.'
            );
        }
    }

    private function _markNotificationAsHandled(Notification $notification): void
    {
        $db   = $this->loris->getDatabaseConnection();
        $stmt = $db->prepare(
            '
            UPDATE redcap_notification as lock_1
            SET
              handled_dt = :handled_dt
            WHERE
              received_dt = :v_received_dt AND
              project_id = :v_project_id AND
              record = :v_record AND
              redcap_event_name = :v_redcap_event_name AND
              instrument = :v_instrument AND
              handled_dt IS NULL
        '
        );

        $now = (new \DateTimeImmutable())->format('Y-m-d H:i:s');

        $notification_params = [
            'v_received_dt'       => $notification->getReceivedDt()->format('Y-m-d H:i:s'),
            'v_project_id'        => $notification->getProjectId(),
            'v_record'            => $notification->getRecord(),
            'v_redcap_event_name' => $notification->getRedcapEventName(),
            'v_instrument'        => $notification->getInstrument(),
            'handled_dt'          => $now
        ];

        $stmt->execute($notification_params);
    }

    private function _releaseNotificationLock(): void
    {
        $db = $this->loris->getDatabaseConnection();
        $db->run('UNLOCK TABLES');
    }
}
