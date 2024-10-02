import {useContext, useEffect, useState} from 'react';
import {Subject, Scanner} from './types';
import {ScannerContext} from './Session';
import { useTranslation } from 'react-i18next';

interface TableProps {
  subject: Subject;
}

/**
 * Table component, which receives its data as props
 *
 * @returns The React element
 */
function Table(props: TableProps) {
  const { t } = useTranslation();
  return (
    <div style={{overflowX: 'scroll'}}>
      <table
        className="table table-hover table-bordered dynamictable"
        id='table-header-left'
      >
        <thead>
          <tr className="info">
            <th>{t('QC Status')}</th>
            <th>{t('Patient Name')}</th>
            <th>{t('PSCID')}</th>
            <th>{t('DCCID')}</th>
            <th>{t('Visit Label')}</th>
            <th>{t('Site')}</th>
            <th>{t('QC Pending')}</th>
            <th>{t('DOB')}</th>
            <th>{t('Sex')}</th>
            <th>{t('Output Type')}</th>
            <th>{t('Scanner')}</th>
            <th>{t('Cohort')}</th>
            {props.subject.useEDC ? <th>{t('EDC')}</th> : null}
          </tr>
        </thead>
        <tbody>
          <tr>
            <td>{props.subject.mriqcstatus}</td>
            <td>
              {props.subject.pscid}
              _
              {props.subject.candid}
              _
              {props.subject.visitLabel}
            </td>
            <td>{props.subject.pscid}</td>
            <td>{props.subject.candid}</td>
            <td>{props.subject.visitLabel}</td>
            <td>{props.subject.site}</td>
            <td>
              {props.subject.mriqcpending === 'Y'
              ? <img
                  src={window.location.origin + '/images/check_blue.gif'}
                  width={12}
                  height={12}
                />
              : null}
            </td>
            <td>{props.subject.dob}</td>
            <td>{props.subject.sex}</td>
            <td>
              {new URLSearchParams(window.location.search).get('outputType')}
            </td>
            <td><Scanner /></td>
            <td>{props.subject.CohortTitle}</td>
            {props.subject.useEDC ? <td>{props.subject.edc}</td> : null}
          </tr>
        </tbody>
      </table>
    </div>
  );
}

/**
 * Scanner component, which reads the scanner context and updates its contenxt
 * once a file has returned with a scanner id
 *
 * @returns The React element
 */
function Scanner() {
  const [scanner, setScanner] = useState<Scanner | null>(null);
  const [scannerID, _] = useContext(ScannerContext);

  useEffect(() => {
    if (scannerID !== null) {
      fetch(window.location.origin
        + `/imaging_browser/getscanner?scannerID=${scannerID}`,
        {credentials: 'same-origin'})
        .then((response) => response.json())
        .then((scanner) => setScanner(scanner));
    }
  }, [scannerID]);

  return (
    <>
      {scanner !== null
        ? `${scanner.manufacturer} ${scanner.model} ${scanner.serialNumber}`
        : null
      }
    </>
  );
}

/**
 * Table component, which fetches data from the module API
 *
 * @returns The React element
 */
function TableWrapper() {
  const [subject, setSubject] = useState<Subject | null>(null);
  const sessionID = new URLSearchParams(window.location.search)
    .get('sessionID');

  useEffect(() => {
    fetch(window.location.origin
      + `/imaging_browser/getsubject?sessionID=${sessionID}`,
      {credentials: 'same-origin'})
      .then((response) => response.json())
      .then((subject) => setSubject(subject));
  }, []);

  return subject ? <Table subject={subject} /> : null;
}

export default TableWrapper;
