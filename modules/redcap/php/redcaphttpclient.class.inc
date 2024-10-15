<?php declare(strict_types=1);

namespace LORIS\redcap;

use \LORIS\LorisInstance;
use \GuzzleHttp\Client;
use \LORIS\Http\Request;
use \LORIS\redcap\Models\IRedcapRecord;
use \LORIS\redcap\Models\RedcapRecord;
use \LORIS\redcap\Models\RedcapRepeatedRecord;
use \LORIS\redcap\Models\RedcapProject;
use \LORIS\StudyEntities\Candidate\CandID;

class RedcapHttpClient
{
    private Client $_client;

    private ?RedcapProject $_project = null;

    private string $_token;

    public function __construct(LorisInstance $loris)
    {
        $settings = $loris->getConfiguration()->getSetting('redcap');

        $this->_token  = $settings['token'];
        $this->_client = new Client(['base_uri' => $settings['url']]);
    }

    /**
     * Return a traversable of IRedcapRecord
     */
    public function getRecords(
        string $pscid,
        string $visit_label,
        string $instrument
    ): \Traversable
    {
        $records   = $this->_fetchCompletedRecord($pscid, $visit_label, $instrument);

        if ($this->isRepeatingInstrument($visit_label, $instrument)) {
            foreach ($records as $index => $record) {
                yield new RedcapRepeatedRecord($instrument, $record, $index + 1);
            }
            return;
        }

        yield new RedcapRecord($instrument, $records[0]);
    }

    public function getRedcapProject(): RedcapProject
    {
        if (is_null($this->_project)) {
            $this->_project = new RedcapProject(
                $this->_fetchProjectInfo()
            );
        }

        return $this->_project;
    }

    public function isRepeatingInstrument(string $visit, string $instrument): bool
    {
        $project = $this->getRedcapProject();

        if (!$project->hasRepeatedInstruments()) {
            return false;
        }

        $repeated = $this->_fetchRepeatingInstruments();

        return array_reduce($repeated, function ($c, $i) use ($visit, $instrument) {
            return $c || ($i['event_name'] == $visit && $i['form_name'] == $instrument);
        }, false);
    }

    /**
     * Returns an ordered list of completed records from REDCap.
     *
     * @param string $pscid The participant id
     * @param string $visit_label The visit label
     * @param string $instrument The instrument name
     */
    private function _fetchCompletedRecord(
        string $pscid,
        string $visit_label,
        string $instrument
    ): array {
        $data = [
            'token'                  => $this->_token,
            'content'                => 'record',
            'action'                 => 'export',
            'format'                 => 'json',
            'type'                   => 'flat',
            'csvDelimiter'           => '',
            'records'                => [$pscid],
            'fields'                 => [],
            'forms'                  => [$instrument],
            'events'                 => [$visit_label],
            'rawOrLabel'             => 'raw',
            'rawOrLabelHeaders'      => 'raw',
            'exportCheckboxLabel'    => 'true',
            'exportSurveyFields'     => 'true',
            'exportDataAccessGroups' => 'true',
            'returnFormat'           => 'json'
        ];

        $response = $this->_client->request(
            'POST',
            '',
            [
                'form_params' => $data,
                'debug'       => false
            ]
        );

        if ($response->getStatusCode() != 200) {
            throw new \LorisException('Can not fetch record');
        }

        $body = json_decode((string) $response->getBody(), true);

        if (empty($body)) {
            throw new \LorisException('No REDCap data found');
        }

        // Only keep complete records
        $completed = array_filter($body, function ($record) use ($instrument) {
            return $record[$instrument . '_complete'] == 2;
        });

        if (count($completed) < 1) {
            throw new \LorisException('No complete record found');
        }

        // Order the records by ${instrument_name}_dtt field value
        usort($completed, function($a, $b) use ($instrument) {
            $a_date = new \DateTimeImmutable($a[$instrument . '_dtt']);
            $b_date = new \DateTimeImmutable($b[$instrument . '_dtt']);
            return $a_date <=> $b_date;
        });

        return $completed;
    }

    private function _fetchRepeatingInstruments(): array
    {
        $data = [
            'token'        => $this->_token,
            'content'      => 'repeatingFormsEvents',
            'format'       => 'json',
            'returnFormat' => 'json'
        ];

        $response = $this->_client->request(
            'POST',
            '',
            [
                'form_params' => $data,
                'debug'       => false
            ]
        );

        $response_string = (string) $response->getBody();
        if ($response_string === 'null') {
            return [];
        }

        return json_decode($response_string, true);
    }

    private function _fetchProjectInfo(): array
    {
        $data = [
            'token'        => $this->_token,
            'content'      => 'project',
            'format'       => 'json',
            'returnFormat' => 'json'
        ];

        $response = $this->_client->request(
            'POST',
            '',
            [
                'form_params' => $data,
                'debug'       => false
            ]
        );

        return json_decode((string) $response->getBody(), true);
    }

    public function fetchEvents(string $pscid): \Traversable
    {
        // TODO :: Fetch from redcap
        // project title
        $db = \NDB_Factory::singleton()->database();
        $candid = $db->pselectOne(
            "SELECT candid FROM candidate WHERE PSCID = :pscid",
            [
                'pscid' => $pscid
            ]
        );

        $project_title = \Candidate::singleton(new CandID(strval($candid)))->getProjectTitle();

        // TODO: Generalize this
        switch ($project_title) {
            case 'HBCD Main':
                yield from [
                    ["event_name"=>"Screening","arm_num"=>1,"unique_event_name"=>"screening_arm_1","custom_event_label"=>null],
                    ["event_name"=>"V01","arm_num"=>1,"unique_event_name"=>"v01_arm_1","custom_event_label"=>null],
                    ["event_name"=>"V02","arm_num"=>1,"unique_event_name"=>"v02_arm_1","custom_event_label"=>null],
                    ["event_name"=>"V03","arm_num"=>1,"unique_event_name"=>"v03_arm_1","custom_event_label"=>null],
                    ["event_name"=>"V04","arm_num"=>1,"unique_event_name"=>"v04_arm_1","custom_event_label"=>null],
                    ["event_name"=>"V05","arm_num"=>1,"unique_event_name"=>"v05_arm_1","custom_event_label"=>null],
                    ["event_name"=>"V06","arm_num"=>1,"unique_event_name"=>"v06_arm_1","custom_event_label"=>null],
                    ["event_name"=>"V07","arm_num"=>1,"unique_event_name"=>"v07_arm_1","custom_event_label"=>null],
                    ["event_name"=>"V07a","arm_num"=>1,"unique_event_name"=>"v07a_arm_1","custom_event_label"=>null],
                    ["event_name"=>"V07b","arm_num"=>1,"unique_event_name"=>"v07b_arm_1","custom_event_label"=>null],
                    ["event_name"=>"V07c","arm_num"=>1,"unique_event_name"=>"v07c_arm_1","custom_event_label"=>null],
                    ["event_name"=>"V08","arm_num"=>1,"unique_event_name"=>"v08_arm_1","custom_event_label"=>null],
                    ["event_name"=>"V09","arm_num"=>1,"unique_event_name"=>"v09_arm_1","custom_event_label"=>null]
                ];
                break;
            case 'HBCD Pilot':
                yield from [
                    ["event_name"=>"Screening","arm_num"=>1,"unique_event_name"=>"screening_arm_1","custom_event_label"=>null],
                    ["event_name"=>"V01","arm_num"=>1,"unique_event_name"=>"v01_arm_1","custom_event_label"=>null],
                    ["event_name"=>"V02","arm_num"=>1,"unique_event_name"=>"v02_arm_1","custom_event_label"=>null],
                    ["event_name"=>"V03","arm_num"=>1,"unique_event_name"=>"v03_arm_1","custom_event_label"=>null],
                    ["event_name"=>"P04","arm_num"=>1,"unique_event_name"=>"v04_arm_1","custom_event_label"=>null],
                    ["event_name"=>"P05","arm_num"=>1,"unique_event_name"=>"v05_arm_1","custom_event_label"=>null],
                    ["event_name"=>"P06","arm_num"=>1,"unique_event_name"=>"v06_arm_1","custom_event_label"=>null],
                    ["event_name"=>"P07a","arm_num"=>1,"unique_event_name"=>"v07a_arm_1","custom_event_label"=>null],
                    ["event_name"=>"P07b","arm_num"=>1,"unique_event_name"=>"v07b_arm_1","custom_event_label"=>null],
                    ["event_name"=>"P07c","arm_num"=>1,"unique_event_name"=>"v07c_arm_1","custom_event_label"=>null]
                ];
                break;
            case 'Montreal Paris NeuroBank':
                yield from [
                    ["event_name" => "TRIAGE", "arm_num" => 1, "unique_event_name" => "triage_arm_1", "custom_event_label" => null]
                ];
                break;
            default:

                die();
                throw new \LorisException('No visit matched to Project');
        }
    }
}
