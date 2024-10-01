<?php declare(strict_types=1);

namespace LORIS\redcap\Endpoints;

use \Psr\Http\Message\ServerRequestInterface;
use \Psr\Http\Message\ResponseInterface;
use \LORIS\Http\Endpoint;
use \LORIS\redcap\Notifications\RedcapNotification;
use \LORIS\redcap\Notifications\RedcapNotificationHandler;
/**
 * This is the handler for redcap notifications
 *
 * @license http://www.gnu.org/licenses/gpl-3.0.txt GPLv3
 */
class Notifications extends Endpoint
{
    /**
     * Return which methods are supported by this endpoint.
     *
     * Projects can only be retrieved, not created.
     *
     * @return array supported HTTP methods
     */
    protected function allowedMethods() : array
    {
        return [
            'POST',
        ];
    }

    /**
     * Handles a request to Resolved conflicts
     *
     * @param ServerRequestInterface $request The incoming PSR7 request
     *
     * @return ResponseInterface
     */
    public function handle(ServerRequestInterface $request) : ResponseInterface
    {
        switch ($request->getMethod()) {
        case 'POST':
            return $this->_handlePOST($request);

        case 'OPTIONS':
            return (new \LORIS\Http\Response())
                ->withHeader('Allow', $this->allowedMethods());

        default:
            return new \LORIS\Http\Response\JSON\MethodNotAllowed(
                $this->allowedMethods()
            );
        }
    }

    /**
     * Handle POST request.
     *
     * @param ServerRequestInterface $request The incoming PSR7 request
     *
     * @return ResponseInterface
     */
    private function _handlePOST(ServerRequestInterface $request): ResponseInterface
    {
        $db = $request->getAttribute('loris')->getDatabaseConnection();

        // Try url-endoded first
        $data = $request->getParsedBody();
        if (empty($data)) {
            // Fallback to application/json
            $data = json_decode((string) $request->getBody(), true);
        }

        $data['received_dt'] = new \DateTimeImmutable();

        try {
            $notification = new RedcapNotification($data);

            // should the notification be ignored?
            //if ($this->ignoreNotification($notification)) {
            //    return new \LORIS\Http\Response();
            //}

            // Add to the database
            $db->insert(
                'redcap_notification',
                $notification->toDatabaseArray()
            );
        } catch (\UnexpectedValueException $e) {
            $body = (string) $request->getBody();
            $this->createIssue(
                $db,
                'Invalid notification received',
                json_encode(
                    [
                        'error' => $e->getMessage(),
                        'data'  => $body
                    ]
                )
            );
            return new \LORIS\Http\Response\JSON\BadRequest($e->getMessage());
        } catch (\DatabaseException $e) {
            $body = (string) $request->getBody();
            $this->createIssue(
                $db,
                'Database exception',
                json_encode(
                    [
                        'error' => $e->getMessage(),
                        'data'  => $body
                    ]
                )
            );
            return new \LORIS\Http\Response\JSON\InternalServerError();
        } catch (\Throwable $e) {
            $body = (string) $request->getBody();
            $this->createIssue(
                $db,
                'Unknown exception',
                json_encode(
                    [
                        'error' => $e->getMessage(),
                        'data'  => $body
                    ]
                )
            );
            return new \LORIS\Http\Response\JSON\InternalServerError();
        }

        $notification_handler = new RedcapNotificationHandler(
            $request->getAttribute('loris')
        );

        try {
            $notification_handler->handle($notification);
        } catch (\DatabaseException $e) {
            $rec = "Error on: "
                . "PSCID: " . $notification->getRecord()
                . "Visit: " . $notification->getRedcapEventName()
                . "instrument: " . $notification->getInstrument();
            $this->createIssue(
                $db,
                'Instrument data not updated - Database exception',
                json_encode(
                    [
                        'error' => $e->getMessage(),
                        'data'  => $rec
                    ]
                )
            );
            return new \LORIS\Http\Response\JSON\InternalServerError();
        } catch (\DomainException $e) {
            $rec = "Error on: "
                . "PSCID: " . $notification->getRecord()
                . "Visit: " . $notification->getRedcapEventName()
                . "instrument: " . $notification->getInstrument();
            $this->createIssue(
                $db,
                'Instrument data not updated - Domain exception',
                json_encode(
                    [
                        'error' => $e->getMessage(),
                        'data'  => $rec
                    ]
                )
            );
            return new \LORIS\Http\Response\JSON\InternalServerError();
        } catch (\Throwable $e) {
            $this->createIssue(
                $db,
                'Instrument data not updated',
                json_encode(
                    [
                        'error' => $e->getMessage(),
                        'data'  => $notification->toDatabaseArray()
                    ]
                )
            );
            return new \LORIS\Http\Response\JSON\InternalServerError();
        }

        return new \LORIS\Http\Response();
    }

    /**
     * Checks if notification should be ignored.
     * Ignored notifications will not trigger any issue creation.
     * Optionally prints an error in log on a case by case basis.
     *
     * @param RedcapNotification $notif the REDCap notification.
     *
     * @return true if the notification should be ignored, else false.
     */
    private function ignoreNotification(RedcapNotification $notif): bool {
        // get authorized instruments list
        $config = \NDB_Factory::singleton()->config();
        $authorizedInstruments = $config->getSetting('redcap_importable_instrument');

        // notification data
        $notificationData = $notif->toDatabaseArray();

        // ignore instruments that are not in the authorized list
        if (!in_array($notif->getInstrument(), $authorizedInstruments, true)) {
            error_log("[redcap] skipped notification, unauthorized instrument: ". json_encode($notificationData));
            return true;
        }

        // ignore notifications that are not 'complete'
        if ($notif->getComplete() != '2') {
            error_log("[redcap] skipped notification, instrument not complete: " . json_encode($notificationData));
            return true;
        }

        // do not ignore
        return false;
    }

    private function createIssue(\Database $db, string $title, string $data)
    {
        // get redcap module ID
        $redcapModuleID = $db->pselectOne(
            'SELECT m.ID FROM modules m WHERE m.Name = "redcap"',
            []
        );

        // add automated centerID for redcap issues
        $centerID = $db->pselectOne(
            'SELECT CenterID FROM psc WHERE Alias="DCC"',
            []
        );

        // insert new issue
        $now = (new \DateTimeImmutable())->format('Y-m-d H:i:s');
        $db->insert('issues', [
            'title'         => $title,
            'reporter'      => 'regis.ongaro-carcy@mcin.ca',
            'assignee'      => 'regis.ongaro-carcy@mcin.ca',
            'status'        => 'assigned',
            'priority'      => 'normal',
            'module'        => $redcapModuleID,
            'dateCreated'   => $now,
            'lastUpdate'    => $now,
            'lastUpdatedBy' => 'regis.ongaro-carcy@mcin.ca',
            'sessionID'     => null,
            'centerID'      => $centerID,
            'candID'        => null,
            'category'      => 'Data Entry',
            'customFields'  => null,
        ]);

        // get inserted issueID
        $issueID = $db->getLastInsertId();

        // add comment
        $db->insert(
            'issues_comments',
            [
                "issueID"       => $issueID,
                "dateAdded"     => $now,
                "addedBy"       => 'regis.ongaro-carcy@mcin.ca',
                "issueComment"  => $data
            ]
        );
    }
}
