-- Create project

INSERT INTO psc (Name, Alias, MRI_alias) VALUES ('Montreal Neurological Institute', 'MNI', 'MNI')
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

-- Create visit

INSERT INTO visit (VisitName, VisitLabel) VALUES ('Q1K', 'Q1K');
SET @visit := LAST_INSERT_ID();
INSERT INTO Visit_Windows (Visit_label) VALUES ('Q1K');
INSERT INTO visit_project_cohort_rel (VisitID, ProjectCohortRelID) VALUES (@visit, @project_cohort);

-- Create MRI protocol group

INSERT INTO mri_protocol_group (Name) VALUES ('Q1K MRI Protocols');
SET @protocol_group := LAST_INSERT_ID();
INSERT INTO mri_protocol_group_target (MriProtocolGroupID, ProjectID) VALUES (@protocol_group, @project);

-- Create scan types

INSERT INTO mri_scan_type (Scan_type) VALUES ('AAHead_Scout_32ch-head-coil');
SET @aahead_scout_32ch_head_coil := LAST_INSERT_ID();
-- There are actually two different scan types with the same name in the specification
INSERT INTO mri_scan_type (Scan_type) VALUES ('fmap-b1_tra_p2_m');
SET @fmap_b1_tra_p2_m := LAST_INSERT_ID();
INSERT INTO mri_scan_type (Scan_type) VALUES ('fmap-b1_tra_p2_flip_angle_map');
SET @fmap_b1_tra_p2_flip_angle_map := LAST_INSERT_ID();
INSERT INTO mri_scan_type (Scan_type) VALUES ('func-cloudy_acq-ep2d_MJC_19mm');
SET @func_cloudy_acq_ep2d_mjc_19mm := LAST_INSERT_ID();
INSERT INTO mri_scan_type (Scan_type) VALUES ('fmap-fmri_acq-mbep2d_SE_19mm_dir-AP');
SET @fmap_fmri_acq_mbep2d_se_19mm_ap := LAST_INSERT_ID();
INSERT INTO mri_scan_type (Scan_type) VALUES ('fmap-fmri_acq-mbep2d_SE_19mm_dir-PA');
SET @fmap_fmri_acq_mbep2d_se_19mm_pa := LAST_INSERT_ID();
INSERT INTO mri_scan_type (Scan_type) VALUES ('func-cross_acq-ep2d_MJC_19mm');
SET @func_cross_acq_ep2d_mjc_19mm := LAST_INSERT_ID();
INSERT INTO mri_scan_type (Scan_type) VALUES ('anat-T1w_acq_mprage_0.8mm_CSptx');
SET @anat_t1w_acq_mp2rage_08mm_csptx := LAST_INSERT_ID();
INSERT INTO mri_scan_type (Scan_type) VALUES ('dwi_acq_multib_38dir_AP_acc9');
SET @dwi_acq_multib_38dir_ap_acc9 := LAST_INSERT_ID();
INSERT INTO mri_scan_type (Scan_type) VALUES ('dwi_acq_multib_70dir_AP_acc9');
SET @dwi_acq_multib_70dir_ap_acc9 := LAST_INSERT_ID();
INSERT INTO mri_scan_type (Scan_type) VALUES ('dwi_acq_b0_PA');
SET @dwi_acq_b0_pa := LAST_INSERT_ID();
INSERT INTO mri_scan_type (Scan_type) VALUES ('anat-T1w_acq-mp2rage_0.7mm_CSptx');
SET @anat_t1w_acq_mp2rage_07mm_csptx := LAST_INSERT_ID();

-- Create MRI protocols

INSERT INTO mri_protocol (Scan_type, MriProtocolGroupID, TR_min, TR_max, TE_min, TE_max, slice_thickness_min, slice_thickness_max, image_type, series_description_regex)
  VALUES
    (@aahead_scout_32ch_head_coil,     @protocol_group, 3.25, 3.25,  1.53, 1.53, 1.6, 1.6, NULL, 'AAHead_Scout_32ch-head-coil'),
    -- TODO: The backslashes seem to not appear in the database for the image types.
    (@fmap_b1_tra_p2_m,                @protocol_group, 4000, 4000,  1.72, 1.72,   4,   4, 'ORIGINAL\PRIMARY\M\ND', NULL),              --  'fmap-b1_tra_p2'
    (@fmap_b1_tra_p2_flip_angle_map,   @protocol_group, 4000, 4000,  1.72, 1.72,   4,   4, 'ORIGINAL\PRIMARY\FLIP ANGLE MAP\ND', NULL), --  'fmap-b1_tra_p2'
    (@func_cloudy_acq_ep2d_mjc_19mm,   @protocol_group, 1720, 1720,  27.8, 27.8, 1.9, 1.9, NULL, 'func-cloudy_acq-ep2d_MJC_19mm'),
    (@fmap_fmri_acq_mbep2d_se_19mm_ap, @protocol_group, 3000, 3000, 11.20, 44.4, 1.9, 1.9, NULL, 'fmap-fmri_acq-mbep2d_SE_19mm_dir-AP'), -- The echo time should be one of 11.20, 27.8, 44.4
    (@fmap_fmri_acq_mbep2d_se_19mm_pa, @protocol_group, 3000, 3000,  18.4, 18.4, 1.9, 1.9, NULL, 'fmap-fmri_acq-mbep2d_SE_19mm_dir-PA'),
    (@func_cross_acq_ep2d_mjc_19mm,    @protocol_group, 1720, 1720, 11.20, 44.4, 1.9, 1.9, NULL, 'func-cross_acq-ep2d_MJC_19mm'), -- The echo time should be one of 11.20, 27.8, 44.4
    (@anat_t1w_acq_mp2rage_08mm_csptx, @protocol_group, 3300, 3300,  2.74, 2.74, 0.8, 0.8, NULL, 'anat-T1w_acq_mprage_0.8mm_CSptx'),
    (@dwi_acq_multib_38dir_ap_acc9,    @protocol_group, 4840, 4840,  79.4, 79.4, 1.1, 1.1, NULL, 'dwi_acq_multib_38dir_AP_acc9'),
    (@dwi_acq_multib_70dir_ap_acc9,    @protocol_group, 4840, 4840,  79.4, 79.4, 1.1, 1.1, NULL, 'dwi_acq_multib_70dir_AP_acc9'),
    (@dwi_acq_b0_pa,                   @protocol_group, 4840, 4840,  79.4, 79.4, 1.1, 1.1, NULL, 'dwi_acq_b0_PA'),
    (@anat_t1w_acq_mp2rage_07mm_csptx, @protocol_group, 5000, 5000,  2.91, 2.91, 0.7, 0.7, NULL, 'anat-T1w_acq-mp2rage_0.7mm_CSptx');

SET @bids_cat_anat := (SELECT BIDSCategoryID FROM bids_category WHERE BIDSCategoryName = 'anat');
SET @bids_cat_dwi  := (SELECT BIDSCategoryID FROM bids_category WHERE BIDSCategoryName = 'dwi');
SET @bids_cat_fmap := (SELECT BIDSCategoryID FROM bids_category WHERE BIDSCategoryName = 'fmap');
SET @bids_cat_func := (SELECT BIDSCategoryID FROM bids_category WHERE BIDSCategoryName = 'func');

SET @bids_subcat_task := (SELECT BIDSScanTypeSubCategoryID FROM bids_scan_type_subcategory WHERE BIDSScanTypeSubCategory = 'task-rest');

SET @bids_type_dwi := (SELECT BIDSScanTypeID FROM bids_scan_type WHERE BIDSScanType = 'dwi');
SET @bids_type_t1w := (SELECT BIDSScanTypeID FROM bids_scan_type WHERE BIDSScanType = 'T1w');
INSERT INTO bids_scan_type (BIDSScanType) VALUES ('q1k_unknown');
SET @bids_type_unknown := LAST_INSERT_ID();

INSERT INTO bids_mri_scan_type_rel (MRIScanTypeID, BidsCategoryID, BIDSScanTypeSubCategoryID, BIDSScanTypeID, BIDSEchoNumber)
   VALUES
    (@aahead_scout_32ch_head_coil,     @bids_cat_anat, @bids_subcat_task, @bids_type_unknown, NULL),
    (@fmap_b1_tra_p2_m,                @bids_cat_fmap, @bids_subcat_task, @bids_type_unknown, NULL),
    (@fmap_b1_tra_p2_flip_angle_map,   @bids_cat_fmap, @bids_subcat_task, @bids_type_unknown, NULL),
    (@func_cloudy_acq_ep2d_mjc_19mm,   @bids_cat_func, @bids_subcat_task, @bids_type_unknown, NULL),
    (@fmap_fmri_acq_mbep2d_se_19mm_ap, @bids_cat_fmap, @bids_subcat_task, @bids_type_unknown, 3),
    (@fmap_fmri_acq_mbep2d_se_19mm_pa, @bids_cat_fmap, @bids_subcat_task, @bids_type_unknown, NULL),
    (@func_cross_acq_ep2d_mjc_19mm,    @bids_cat_func, @bids_subcat_task, @bids_type_unknown, 3),
    (@anat_t1w_acq_mp2rage_08mm_csptx, @bids_cat_anat, @bids_subcat_task, @bids_type_t1w, NULL),
    (@dwi_acq_multib_38dir_ap_acc9,    @bids_cat_dwi,  @bids_subcat_task, @bids_type_dwi, NULL),
    (@dwi_acq_multib_70dir_ap_acc9,    @bids_cat_dwi,  @bids_subcat_task, @bids_type_dwi, NULL),
    (@dwi_acq_b0_pa,                   @bids_cat_dwi,  @bids_subcat_task, @bids_type_dwi, NULL),
    (@anat_t1w_acq_mp2rage_07mm_csptx, @bids_cat_anat, @bids_subcat_task, @bids_type_t1w, NULL);
