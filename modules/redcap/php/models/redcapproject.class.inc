<?php declare(strict_types=1);

namespace LORIS\redcap\Models;

/**
 * This reprensents a redcap record.
 *
 * @license http://www.gnu.org/licenses/gpl-3.0.txt GPLv3
 */
class RedcapProject
{
    private array $_props;

    /**
     * Constructor
     *
     * @param array $props The project properties typically from redcap.
     */
    public function __construct(array $props)
    {
        $this->_props = $props;
    }

    public function hasRepeatedInstruments(): bool
    {
        return ($this->_props['has_repeating_instruments_or_events'] == 1);
    }
}
