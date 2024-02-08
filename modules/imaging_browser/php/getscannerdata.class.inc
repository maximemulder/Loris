<?php

namespace LORIS\imaging_browser;

use \Psr\Http\Message\ServerRequestInterface;
use \Psr\Http\Server\RequestHandlerInterface;
use \Psr\Http\Message\ResponseInterface;

/**
 * Page that returns a scanner's data as a Json.
 *
 * @license http://www.gnu.org/licenses/gpl-3.0.txt GPLv3
 */
class GetScannerData extends \NDB_Page implements RequestHandlerInterface
{
    /**
     * The database connection
     *
     * @var \Database
     */
    private $db;

    /**
     * This function passes the request to the handler. This is necessary since
     * the Endpoint bypass the Module class.
     *
     * XXX: This function should be extracted to a parent class.
     *
     * @param ServerRequestInterface  $request The PSR7 request.
     * @param RequestHandlerInterface $handler The request handler.
     *
     * @return ResponseInterface The outgoing PSR7 response.
     */
    public function process(
        ServerRequestInterface $request,
        RequestHandlerInterface $handler
    ) : ResponseInterface {
        return $handler->handle($request);
    }

    /**
     * This acts as an Ajax enpoint that handles all action requests from the
     * Battery Manager Module.
     *
     * @param ServerRequestInterface $request The incoming PSR7 request
     *
     * @return ResponseInterface The outgoing PSR7 response
     */
    public function handle(ServerRequestInterface $request) : ResponseInterface
    {
        $this->db = $this->loris->getDatabaseConnection();
        $method   = $request->getMethod();

        switch ($method) {
        case 'GET':
            return $this->_getData();
        }
    }

    /**
     * Returns a scanner's data as a Json response
     *
     * @return ResponseInterface The Json response.
     */
    private function _getData() : ResponseInterface
    {
        $row = $this->_getRow($_REQUEST['scannerID']);
        return new \LORIS\Http\Response\JSON\OK($row);
    }

    /**
     * Returns a scanner's data as a Json response
     *
     * @param $scannerID The scanner's id
     *
     * @return array The Json response.
     */
    private function _getRow($scannerID)
    {
        $row = $this->db->pselectRow(
            "SELECT ID, Manufacturer, Model, Serial_number
            FROM mri_scanner WHERE ID=:ScannerID",
            ['ScannerID' => $scannerID]
        );

        return [
            'id'           => (int) $row['ID'],
            'manufacturer' => $row['Manufacturer'],
            'model'        => $row['Model'],
            'serialNumber' => $row['Serial_number'],
        ];
    }
}