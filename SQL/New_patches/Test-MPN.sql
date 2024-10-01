-- Create project

INSERT INTO psc (Name, Alias, MRI_alias) VALUES ('Montreal Neurological Institute', 'MNI', 'MNI');
SET @site := LAST_INSERT_ID();

INSERT INTO Project (Name, Alias) VALUES ('Montreal Paris NeuroBank', 'MPN');
SET @project := LAST_INSERT_ID();

INSERT INTO cohort (title) VALUES ('Control');
SET @cohort := LAST_INSERT_ID();

INSERT INTO project_cohort_rel (ProjectID, CohortID) VALUES (@project, @cohort);
SET @project_cohort := LAST_INSERT_ID();

-- Associate admin with project

SET @admin := (SELECT ID FROM users WHERE UserID = 'admin');
INSERT INTO user_project_rel (UserID, ProjectID) VALUES (@admin, @project);
INSERT INTO user_psc_rel (UserID, CenterID) VALUES (@admin, @site);

-- Add visits

INSERT INTO visit (VisitName, VisitLabel) VALUES ('TRIAGE', 'TRIAGE');
SET @visit_triage := LAST_INSERT_ID();
INSERT INTO visit_project_cohort_rel (VisitID, ProjectCohortRelID) VALUES (@visit_triage, @project_cohort);
