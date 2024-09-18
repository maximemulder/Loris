-- INSERT INTO `test_subgroups` (`Subgroup_name`) VALUES ('MPN Instruments');
-- SET @subgroup_id := LAST_INSERT_ID();
SET @subgroup_id := 3;

INSERT INTO `test_names` (`Test_name`, `Full_name`, `Sub_group`) VALUES
  ('exclusion_criteria'                                 , 'Exclusion criteria'                             , @subgroup_id),
  ('demographics'                                       , 'Demographics'                                   , @subgroup_id),
  ('general_medical_profile'                            , 'General medical profile'                        , @subgroup_id),
  ('handedness'                                         , 'Handedness'                                     , @subgroup_id),
  ('patient_health_questionnaire_9'                     , 'Patient health questionnaire'                   , @subgroup_id),
  ('alcohol_use_disorders_identification_testconcise_a' , 'Alcohol use disorders identification'           , @subgroup_id),
  ('fagerstrm_test_for_nicotine_dependence_ftnd'        , 'Test for nicotine dependence'                   , @subgroup_id),
  ('drug_abuse_screening_test_dast10'                   , 'Drug abuse screening'                           , @subgroup_id),
  ('satisfaction_with_life_scale_swls'                  , 'Satisfaction with life scale'                   , @subgroup_id),
  ('big_five10'                                         , 'Big five'                                       , @subgroup_id),
  ('general_self_efficacy'                              , 'General self efficacy'                          , @subgroup_id),
  ('rand36_quality_of_life'                             , 'Quality of life'                                , @subgroup_id),
  ('generalized_anxiety_disorder_7_item_gad_7_scale'    , 'Generalized anxiety disorder scale'             , @subgroup_id),
  ('pittsburg_sleep_quality_index_psqi'                 , 'Pittsburg sleep quality index'                  , @subgroup_id),
  ('epworths_sleepiness_scale'                          , 'Epworths sleeepines scale'                      , @subgroup_id),
  ('meaning_of_life_questionnaire'                      , 'Meaning of life questionnaire'                  , @subgroup_id),
  ('perceived_stress_questionnaire'                     , 'Perceived stress questionnaire'                 , @subgroup_id),
  ('social_participation_questionnaire_ccna'            , 'Social participation questionnaire'             , @subgroup_id),
  ('ucla_loneliness3'                                   , 'UCLA Loneliness'                                , @subgroup_id),
  ('physical_activity'                                  , 'Physical activity'                              , @subgroup_id),
  ('the_cognitive_leisure_activity_scale_clas'          , 'The cognitive lesure actvity scale'             , @subgroup_id),
  ('d_curiosity_scale'                                  , 'Curiosity scale'                                , @subgroup_id),
  ('cognitive_reserve_index'                            , 'Cognitive reserve'                              , @subgroup_id),
  ('screen_usage'                                       , 'Screen usage'                                   , @subgroup_id),
  ('music_history'                                      , 'Music history'                                  , @subgroup_id),
  ('incomplete_language_experience_and_proficiency_que' , 'Incomplete language experience and proficiency' , @subgroup_id),
  ('minors_ses_questionnaire'                           , 'Minors SES questionnaire'                       , @subgroup_id),
  ('puberty_scale'                                      , 'Puberty scale'                                  , @subgroup_id),
  ('moca'                                               , 'MOCA'                                           , @subgroup_id);
