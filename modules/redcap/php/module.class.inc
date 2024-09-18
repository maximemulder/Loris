<?php declare(strict_types=1);

namespace LORIS\redcap;

use \Psr\Http\Message\ServerRequestInterface;
use \Psr\Http\Message\ResponseInterface;

/**
 * This sets specific bahaviours or the redcap module and provides
 * the initial request routing.
 *
 * @license http://www.gnu.org/licenses/gpl-3.0.txt GPLv3
 */
class Module extends \Module
{
    /**
     * This needs to be true since redcap wont authenticate.
     *
     * @return bool True
     */
    public function isPublicModule() : bool
    {
        return true;
    }

    /**
     * Return the long name of this module.
     *
     * @return string
     */
    public function getLongName() : string
    {
        return "RedCap";
    }

    /**
     * Perform routing for this module's endpoints.
     *
     * @param ServerRequestInterface $request The incoming PSR7 request
     *
     * @return ResponseInterface the outgoing PSR7 response
     */
    public function handle(ServerRequestInterface $request) : ResponseInterface
    {
        $request_data = [
            'GET'     => $_GET,
            'POST'    => $_POST,
            'REQUEST' => $_REQUEST,
            'SERVER'  => $_SERVER,
            'FILES'   => $_FILES,
            'COOKIES' => $_COOKIE,
        ];

        // Log the request to a temporary file
        $request_string = print_r($request_data, TRUE);

        $fp = fopen('/data/tmp/redcap_request.log', 'a');
        fwrite($fp, $request_string);
        fclose($fp);

        $path = trim($request->getURI()->getPath(), "/");
        switch ($path) {
        case 'notifications':
            $handler = new Endpoints\Notifications($this->loris);
            break;
        case 'dev_test':
            $body = (new Notifications\RedcapNotificationHandler(
                $request->getAttribute('loris')
            ))->test();
            return new \LORIS\Http\Response(new \LORIS\Http\StringStream($body));
        default:
            return parent::handle($request);
        }

        return $handler->process($request, $handler);
    }

    /**
     * {@inheritDoc}
     *
     * @return bool Whether an api schema is available fro this module.
     */
    public function hasAPISpecification(): bool
    {
        return true;
    }
}

