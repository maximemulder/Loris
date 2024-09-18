<?php declare(strict_types=1);

namespace LORIS\redcap\Notifications;

/**
 * This reprensents a redcap notification.
 *
 * @license http://www.gnu.org/licenses/gpl-3.0.txt GPLv3
 */
class RedcapNotification
{
    private string $instrument;
    private string $project_id;
    private string $project_url;
    private \DateTimeInterface $received_dt;
    private string $record;
    private string $redcap_event_name;
    private string $redcap_url;
    private string $complete;
    private string $username;

    /**
     * Constructor
     *
     * @param array $props The notification properties typically from redcap.
     */
    public function __construct(array $props)
    {
        $requiredprops = [
            'instrument',
            'project_id',
            'project_url',
            'received_dt',
            'record',
            'redcap_event_name',
            'redcap_url',
            'username',
        ];

        // Make sure all the required properties exists
        foreach ($requiredprops as $required) {
            if (empty($props[$required] ?? null)) {
                throw new \UnexpectedValueException('Missing property ' . $required);
            }
        }

        // ${instrument}_complete is required as well.
        $complete_key = $props['instrument'] . '_complete';
        if (!isset($props[$complete_key])) {
            throw new \UnexpectedValueException('Missing property ' . $complete_key);
        }

        $this->instrument        = $props['instrument'];
        $this->project_id        = $props['project_id'];
        $this->project_url       = $props['project_url'];
        $this->received_dt       = $props['received_dt'];
        $this->record            = $props['record'];
        $this->redcap_event_name = $props['redcap_event_name'];
        $this->redcap_url        = $props['redcap_url'];
        $this->complete          = $props[$complete_key] ?? '';
        $this->username          = $props['username'];
    }

    /**
     * Getter for the complete property
     *
     * @return string
     */
    public function getComplete(): string
    {
        return $this->complete;
    }

    /**
     * Getter for the instrument property
     *
     * @return string
     */
    public function getInstrument(): string
    {
        return $this->instrument;
    }

    /**
     * Getter for the project_id property
     *
     * @return string
     */
    public function getProjectId(): string
    {
        return $this->project_id;
    }

    /**
     * Getter for the complete property
     *
     * @return string
     */
    public function getRecord(): string
    {
        return $this->record;
    }

    /**
     * Getter for the received_dt property
     *
     * @return \DateTimeInterface
     */
    public function getReceivedDt(): \DateTimeInterface
    {
        return $this->received_dt;
    }

    /**
     * Getter for the redcap_event_name property
     *
     * @return string
     */
    public function getRedcapEventName(): string
    {
        return $this->redcap_event_name;
    }

    /**
     * Creates a database row representation of this notification.
     *
     * @return array
     */
    public function toDatabaseArray(): array
    {
        return [
            'instrument'        => $this->instrument,
            'project_id'        => $this->project_id,
            'project_url'       => $this->project_url,
            'received_dt'       => $this->received_dt->format('Y-m-d H:i:s'),
            'record'            => $this->record,
            'redcap_event_name' => $this->redcap_event_name,
            'redcap_url'        => $this->redcap_url,
            'complete'          => $this->complete,
            'username'          => $this->username,
        ];
    }
}
