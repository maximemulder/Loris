-- Create project

INSERT INTO psc (Name, Alias, MRI_alias) VALUES ('Montreal Neurological Institute', 'MNI', 'MNI');
-- SET @site := (SELECT CenterID FROM psc WHERE Name = 'Montreal Neurological Institute');
SET @site := LAST_INSERT_ID();

INSERT INTO Project (Name, Alias) VALUES ('Montreal Paris NeuroBank', 'MPN');
-- SET @project := (SELECT ProjectID FROM Project WHERE Name = 'Montreal Paris NeuroBank');
SET @project := LAST_INSERT_ID();

INSERT INTO cohort (title) VALUES ('Control');
-- SET @cohort := (SELECT CohortID FROM cohort WHERE title = 'Control');
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

INSERT INTO visit (VisitName, VisitLabel) VALUES ('Post enrolment', 'Post_enrolment');
SET @visit_post_enrolment := LAST_INSERT_ID();
INSERT INTO visit_project_cohort_rel (VisitID, ProjectCohortRelID) VALUES (@visit_post_enrolment, @project_cohort);

-- Add candidate
-- INSERT INTO `candidate` (`CandID`, `PSCID`, `ExternalID`, `DoB`, `DoD`, `EDC`, `Sex`, `RegistrationCenterID`, `RegistrationProjectID`, `Ethnicity`, `Active`, `Date_active`, `RegisteredBy`, `UserID`, `Date_registered`, `flagged_caveatemptor`, `flagged_reason`, `flagged_other`, `flagged_other_status`, `Testdate`, `Entity_type`, `ProbandSex`, `ProbandDoB`)
--   VALUES (123456, '1', NULL, '1998-08-27', NULL, NULL, 'Male', @site, @project, NULL, 'Y', '2024-01-15', NULL, 'admin','2024-01-15', 'false', NULL, NULL, NULL, '2024-09-30 10:00:00','Human', NULL, NULL);

INSERT INTO `examiners` (`full_name`) VALUES ('REDCap');
