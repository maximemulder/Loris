import {useLayoutEffect, useRef, useState} from 'react';

declare const loris: any;
declare const Help: any;

interface Categories {
  [keyid: string]: [{label: string, link: string}];
};

interface LogoProps {
  sandbox: boolean;
}

function Logo(props: LogoProps) {
  return (
    <a className="loris-header-logo" href={loris.BaseURL}>
      LORIS{props.sandbox ? ': DEV' : ''}
    </a>
  );
}

interface CategoriesProps {
  categories: Categories;
}

function Categories(props: CategoriesProps) {
  return (
    <nav className="loris-header-categories">
      {Object.entries(props.categories).map(([category, modules]) => (
        <div key={category} className="loris-header-category">
          <div className="loris-header-title">
              {category} <span className="caret" />
          </div>
          <ul className="loris-header-modules">
            {modules.map((module) => (
              <li key={module.label}>
                <a href={module.link} className="loris-header-module">
                  {module.label}
                </a>
              </li>
            ))}
          </ul>
        </div>
      ))}
    </nav>
  );
}

interface ControlsProps {
  feedbackPanel: boolean;
}

function Controls(props: ControlsProps) {
  return (
    <div className="loris-header-controls">
      {props.feedbackPanel ? (
        <div className="loris-header-control navbar-toggle" data-toggle="offcanvas" data-target=".navmenu" data-canvas="body">
          <span className="sr-only">Toggle navigation</span>
          <div className="glyphicon glyphicon-edit" />
        </div>
      ) : null}
      <div id="help-container">
        <Help
          testname={loris.TestName}
          subtest={loris.Subtest}
          baseURL={loris.BaseURL}
        />
      </div>
    </div>
  );
}

interface InfosProps {
  name: string;
  prefs: boolean;
  sitesCount: number;
  sitesTooltip: string;
}

function Infos(props: InfosProps) {
  return (
    <div className="loris-header-infos">
      <div className="loris-header-info css-tooltip">
          <div className="loris-header-title">
            Site Affiliations: {props.sitesCount}
          </div>
          <span className="tooltip-text">{props.sitesTooltip}</span>
      </div>
      <div className="loris-header-info dropdown">
        <div className="loris-header-title dropdown-toggle" data-toggle="dropdown">
            {props.name} <span className="caret" />
        </div>
        <div className="dropdown-menu">
          {props.prefs ? (
            <a href={loris.BaseURL + '/my_preferences'}>
              My Preferences
            </a>
          ) : null}
          <a href={loris.BaseURL + '/?logout=true'}>
            Log Out
          </a>
        </div>
      </div>
    </div>
  );
}

interface HeaderProps {
  sandbox: boolean;
  categories: Categories;
  controlPanel: boolean;
  feedbackPanel: boolean;
  userName: string;
  userPrefs: boolean;
  sitesCount: number;
  sitesTooltip: string;
}

function Header(props: HeaderProps) {
  const refHeader = useRef<HTMLDivElement>(null);
  const refFill = useRef<HTMLDivElement>(null);
  const [contentWidth, setContentWidth] = useState<number | null>(null);
  const [compact, setCompact] = useState<boolean>(false);

  useLayoutEffect(() => {
    if (refHeader.current !== null && refFill.current !== null) {
      setContentWidth(refHeader.current.scrollWidth - refFill.current.offsetWidth);
    }
  }, [refHeader, refFill]);

  useLayoutEffect(() => {
    function update() {
      if (contentWidth !== null) {
        setCompact((refHeader.current as HTMLDivElement).offsetWidth < contentWidth);
      }
    }

    window.addEventListener('resize', update);
    update();
    return () => window.removeEventListener('resize', update);
  }, [contentWidth]);

  return (
    <div
      className={'loris-header' + (compact ? ' loris-header-fold' : '')}
      ref={refHeader}
    >
      {props.controlPanel ? (
        <div id="menu-toggle" className="loris-header-sidebar">
          <span className="glyphicon glyphicon-th-list" />
        </div>
      ) : null}
      <Logo sandbox={props.sandbox} />
      <Categories categories={props.categories} />
      <div className="loris-header-fill" ref={refFill} />
      <Controls feedbackPanel={props.feedbackPanel} />
      <Infos
        name={props.userName}
        prefs={props.userPrefs}
        sitesCount={props.sitesCount}
        sitesTooltip={props.sitesTooltip}
      />
    </div>
  );
}

(window as any).Header = Header;

/*

{* I dunno what that is *}
<button type="button" class="navbar-toggle collapsed nav-button" data-toggle="collapse"
    data-target="#example-navbar-collapse">
    <span class="sr-only">Toggle navigation</span>
    <span class="toggle-icon glyphicon glyphicon-chevron-down" style="color:white"></span>
</button>
*/
