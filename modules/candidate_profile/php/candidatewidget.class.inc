<?php declare(strict_types=1);

namespace LORIS\candidate_profile;

/**
 * A \LORIS\candidate_profile\CandidateWidget is a type of \LORIS\GUI\Widget
 * used by the candidate dashboard.
 *
 * All CandidateWidgets consist of React components which are loaded on the fly.
 * The React component can have arbitrary props sent to it from LORIS.
 *
 * CandidateWidgets also have an optional width/height/order which position
 * it in the CSSGrid.
 *
 * @license http://www.gnu.org/licenses/gpl-3.0.txt GPLv3
 */
class CandidateWidget implements \LORIS\GUI\Widget
{
    private string $_title;
    private string $_url;
    private string $_componentname;
    private array $_props;

    private ?int $_width;
    private ?int $_height;
    private ?int $_order;

    /**
     * Construct a dashboard widget with the specified properties.
     *
     * @param string $title         The title of the card to display.
     * @param string $jsurl         The URL containing the React component.
     * @param string $componentname The React component name for this widget.
     * @param array  $props         Additional React props to pass to the React
     *                              component.
     * @param ?int   $width         The width in the CSS grid.
     * @param ?int   $height        The height in the CSS grid.
     * @param ?int   $order         The order in the CSS grid.
     */
    public function __construct(
        string $title,
        string $jsurl,
        string $componentname,
        array $props,
        ?int $width = null,
        ?int $height = null,
        ?int $order = null
    ) {
        $this->_title         = $title;
        $this->_url           = $jsurl;
        $this->_width         = $width;
        $this->_height        = $height;
        $this->_componentname = $componentname;
        $this->_props         = $props;
        $this->_order         = $order;
    }

    /**
     * Renders the widget within a dashboard panel and implements
     * the \LORIS\GUI\Widget interface.
     *
     * @return string the HTML content of the widget to be rendered
     */
    public function __toString()
    {
        return $this->_url;
    }

    /**
     * Return the Card title
     *
     * @return string
     */
    public function getTitle() : string
    {
        return $this->_title;
    }

    /**
     * Return the Card width.
     *
     * @return ?int
     */
    public function getWidth() : ?int
    {
        return $this->_width;
    }

    /**
     * Return the Card height.
     *
     * @return ?int
     */
    public function getHeight() : ?int
    {
        return $this->_height;
    }

    /**
     * Return the Card order.
     *
     * @return ?int
     */
    public function getOrder() : ?int
    {
        return $this->_order;
    }

    /**
     * Return the URL which contains the React
     * component for the Card's body.
     *
     * @return string
     */
    public function getJSURL() : string
    {
        return $this->_url;
    }

    /**
     * Return the name of the React component to
     * render.
     *
     * @return string
     */
    public function getComponentName() : string
    {
        return $this->_componentname;
    }

    /**
     * Return additional React props to pass to the
     * React component
     *
     * @return array
     */
    public function getComponentProps() : array
    {
        return $this->_props;
    }
}
