import {MouseEvent, ReactNode} from 'react';
import {ImageFiles} from './types';
import { useTranslation } from 'react-i18next';

interface ButtonProps {
  icon: string;
  children: ReactNode;
  url?: string;
  onClick?: (e: MouseEvent) => void;
}

/**
 * Generic clickable button component, which may be a link or not
 *
 * @returns The React element
 */
function Button(props: ButtonProps) {
  /**
   * Higher-order component for the button
   *
   * @returns The React element
   */
  const element = (children: ReactNode) => props.url ?
    (
      <a
        href={props.url}
        className="btn btn-default"
        onClick={props.onClick}
        style={{margin: 0}}
      >
        {children}
      </a>
    ) : (
      <div
        className="btn btn-default"
        onClick={props.onClick}
        style={{margin: 0}}
      >
        {children}
      </div>
    );

  return element(
    <>
      <span
        className={`glyphicon glyphicon-${props.icon}`}
        style={{marginRight: '5px'}}
      />
      <span className="hidden-xs">{props.children}</span>
    </>
  );
}

interface LongitudinalViewButtonProps {
  OtherTimepoints: number,
}

/**
 * Image Longitudinal View Button component
 *
 * @returns The React element
 */
function LongitudinalViewButton(props: LongitudinalViewButtonProps) {
  const { t } = useTranslation();

  const url = window.location.origin
    + `/brainbrowser/?minc_id=${props.OtherTimepoints}`;

  /**
   * Open brain browser handler
   *
   * @returns void
   */
  const openWindowHandler = (e: MouseEvent) => {
    e.preventDefault();
    window.open(
      url,
      'BrainBrowser Volume Viewer',
      'location = 0,width = auto, height = auto, scrollbars=yes'
    );
  };

  return (
    <Button icon="eye-open" url={url} onClick={openWindowHandler}>
      {t('Longitudinal View')}
    </Button>
  );
}

interface QcButtonProps {
  FileID: number;
}

/**
 * Image Quality Control Comments Button component
 *
 * @returns The React element
 */
function QcButton(props: QcButtonProps) {
  const { t } = useTranslation();

  const url = window.location.origin
    + `/imaging_browser/feedback_mri_popup/fileID=${props.FileID}`;

  /**
   * Open feedback handler
   *
   * @returns void
   */
  const openWindowHandler = (e: MouseEvent) => {
    e.preventDefault();
    window.open(
      url,
      'feedback_mri',
      'width=700,height=800,toolbar=no,location=no,'
      + 'status=yes,scrollbars=yes,resizable=yes'
    );
  };

  return (
    <Button icon="pencil" url={url} onClick={openWindowHandler}>
      {t('QC Comments')}
    </Button>
  );
}

interface HeadersButtonProps {
  headersExpanded: boolean;
  toggleHeaders?: () => void;
}

/**
 * Image Longitudinal View Button
 *
 * @returns The React element
 */
function HeadersButton(props: HeadersButtonProps) {
  const { t } = useTranslation();
  return (
    <Button icon="th-list" onClick={props.toggleHeaders}>
      {!props.headersExpanded ? t('Show Headers') : t('Hide Headers')}
    </Button>
  );
}

interface DownloadButtonProps {
  label: string,
  url?: string,
  fileName?: string,
}

/**
 * Download button component
 *
 * One of the `url` or `fileName` prop must be defined.
 *
 * @returns The React element
 */
function DownloadButton(props: DownloadButtonProps) {
  const url = props.url
    || `${window.location.origin}/mri/jiv/get_file.php?file=${props.fileName}`;
  return (
    <Button icon="download-alt" url={url}>
      {props.label}
    </Button>
  );
}

interface ImageButtonsProps {
  FileID: number;
  APIFile: string;
  OtherTimepoints: number;
  files: ImageFiles;
  headersExpanded: boolean;
  toggleHeaders?: () => void;
}

/**
 * The Image Buttons component
 *
 * @returns The React element
 */
function ImageButtons(props: ImageButtonsProps) {
  const { t } = useTranslation();

  const style = {
    display: 'flex' as const,
    alignItems: 'center' as const,
    flexWrap: 'wrap' as const,
    gap: '5px',
    paddingBottom: '10px',
  };

  return (
    <div style={style}>
      <LongitudinalViewButton
        OtherTimepoints={props.OtherTimepoints}
      />
      <QcButton
        FileID={props.FileID}
      />
      <HeadersButton
        headersExpanded={props.headersExpanded}
        toggleHeaders={props.toggleHeaders}
      />
      <DownloadButton
        url={props.APIFile}
        label={t('Download Image')}
      />
      { props.files.protocol ?
        <DownloadButton
          fileName={props.files.protocol}
          label={t("Download XML Protocol")}
        /> : null
      }
      { props.files.report ?
        <DownloadButton
          fileName={props.files.report}
          label={t("Download XML Report")}
        /> : null
      }
      { props.files.nrrd ?
        <DownloadButton
          fileName={props.files.nrrd}
          label={t("Download NRRD")}
        /> : null
      }
      { props.files.nii ?
        <DownloadButton
          url={props.APIFile + '/format/nifti'}
          label={t("Download NIfTI")}
        /> : null
      }
      { props.files.bval ?
        <DownloadButton
          url={props.APIFile + '/format/bval'}
          label={t("Download BVAL")}
        /> : null
      }
      { props.files.bvec ?
        <DownloadButton
          url={props.APIFile + '/format/bvec'}
          label={t("Download BVEC")}
        /> : null
      }
      { props.files.json ?
        <DownloadButton
          url={props.APIFile + '/format/bidsjson'}
          label={t("Download BIDS JSON")}
        /> : null
      }
    </div>
  );
}

export default ImageButtons;
