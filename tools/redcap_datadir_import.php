#!/usr/bin/env php
<?php

require_once __DIR__ . "/generic_includes.php";

$options = getopt(
    "f:r:",
    [
        "output-dir:",
        "file:",
        "redcap-api:",
        "redcap-token:",
        "trim-formname",
    ]
);

$inputFile    = $options['f'] ?? $options['file'] ?? '';
$redcapAPIURL = $options['r'] ?? $options['redcap-api'] ?? '';
$redcapToken  = $options['redcap-token'] ?? '';
$outputDir    = $options['output-dir'] ?? '';
$trimName     = isset($options['trim-formname']);

if (empty($inputFile) && empty($redcapAPIURL)) {
    fprintf(STDERR, "Either --file or --redcap-api must be provided.\n\n");
    showHelp();
    exit(1);
}
if (!empty($redcapAPIURL) && empty($options['redcap-token'])) {
    fprintf(STDERR, "Either --redcap-api specified but missing redcap-token.\n\n");
    showHelp();
    exit(1);
}
if (empty($outputDir)) {
    showHelp();
    exit(1);
}
if (!is_dir($outputDir)) {
    fprintf(STDERR, "Output directory $outputDir doesn't exist.\n");
    exit(1);
}
if (!is_writeable($outputDir)) {
    fprintf(STDERR, "Output directory $outputDir is not writeable.\n");
    exit(1);
}

// REDCap only instrument that are not needed on LORIS.
$REDCapOnlyInstrument = [
    'setup',
    'setup_l',
    'remote_survey_introduction',
    'child_survey_introduction',
    'parent_survey_introduction',
    'family_surveys_introduction',
    'sensitive_question_introduction',
    'remote_survey_completion',
    # these are cut from REDCap and reimplemented in LORIS
    'admin_part_fb', # renamed 'adm_cg_fb' in loris
    'admin_alert',
];

$fp = getDictionaryCSVStream($redcapAPIURL, $redcapToken, $inputFile);

fwrite(STDERR, "\n-- Parsing files\n\n");

$headers     = fgetcsv($fp);
$instruments = [];
$badMap = 0;
$mapped = 0;
$lastREDCapError = '';
while ($row = fgetcsv($fp)) {
    $inst = $row[1];
    // skip REDCap only instruments
    if (array_search($inst, $REDCapOnlyInstrument) !== false) {
        $msg = " -> REDCap only instrument '$inst' skipped.\n";
        // to avoid repeating same msg
        if ($lastREDCapError !== $msg) {
            $lastREDCapError = $msg;
            fwrite(STDERR, $msg);
        }
        continue;
    }
    //
    if (!isset($instruments[$inst])) {
        $instruments[$inst] = [];
    }
    $fieldname = $row[0];
    if ($trimName) {
        $formname = $row[1];
        if (strpos($fieldname, $formname) !== 0) {
            $badMap++;
            fwrite(STDERR, "Field [$fieldname] does not have form name '$formname' as a prefix\n");
        } else {
            // debug
            //$oldname = $fieldname;
            $mapped++;
            $fieldname = preg_replace("/^" . preg_quote($formname) . "(_*)/", "", $fieldname);
            // fwrite(STDERR, "Field $oldname became $fieldname\n");
        }
    }

    $linstFormat = toLINST($row[3], $fieldname, $row[4], $row[5]);
    if (!empty($linstFormat)) {
        $instruments[$inst][] = $linstFormat;
    }
}
fclose($fp);

if ($trimName) {
    fwrite(STDERR, "\nCould not map $badMap fields\nMapped $mapped fields\n");
}

outputFiles($outputDir, $instruments, getTestNameMapping($redcapAPIURL, $redcapToken));

/**
 * Take a single line from the redcap dictionary and returns the
 * closest LINST equivalent.
 *
 * @param string $redcaptype      The type from REDCap
 * @param string $redcapfieldname The fieldname from REDCap
 * @param string $redcaplabel     The label from REDCap
 * @param string $redcapChoices   The choices columnfrom REDCap
 *
 * @return string
 */
function toLINST(
    string $redcaptype,
    string $redcapfieldname,
    string $redcaplabel,
    string $redcapChoices,
) : string {
    $label = str_replace("\n", "<br /><br />", $redcaplabel);
    switch ($redcaptype) {
    case 'text':
        // text maps directly to LORIS
        return "text{@}$redcapfieldname{@}$label";
    case 'descriptive':
        // descriptive maps to label with no field.
        return "static{@}{@}$label";
    case 'radio':
    case 'dropdown':
        // Radio or dropdown maps to a select and the options are in the
        // same format in the dictionary.
        $selectoptions = optionsToLINST($redcapChoices);
        if (!empty($selectoptions)) {
            $selectoptions = "NULL=>''{-}" . $selectoptions;
        }
        return "select{@}$redcapfieldname{@}$label{@}$selectoptions";
    case 'checkbox':
        // checkboxes are the same format as radios but allow multiple options,
        // so map to a selectmultiple instead of a select
        return "selectmultiple{@}$redcapfieldname{@}$label{@}"
            . optionsToLINST($redcapChoices);
    case 'yesno':
        // Map yes/no fields to dropdowns with yes and no options.
        return "select{@}$redcapfieldname{@}$label{@}"
            . "NULL=>''{-}'yes'=>'Yes'{-}'no'=>'No'";
    case 'calc':
        // Calc maps to a score field. We create the DB field but don't do the score.
        return "static{@}$redcapfieldname{@}$label";
    case 'sql':
        // The "SQL" data type is used for a dynamic display of enum options. Since
        // we don't have access to the redcap database that the sql is selecting from,
        // we treat it the same way as a score/calc field so that data can be imported.
        return "static{@}$redcapfieldname{@}$label";
    case'slider':
        return "numeric{@}$redcapfieldname{@}$label{@}0{@}100";
    case 'file':
        // File upload - NOT SUPPORTED BY LINST
        return "";
    case 'notes':
        // REDCap calls textareas notes
        return "textarea{@}$redcapfieldname{@}$label";
    default:
        throw new \LorisException("Unhandled REDCap type $redcaptype");
    }
}

/**
 * Take the options column from a dictionary line and convert it to the
 * linst format for select/multiselect
 *
 * @param string $dictionary The dictionary from REDCap
 *
 * @return string
 */
function optionsToLINST(string $dictionary) : string
{
    $dictionary = str_replace(' | | ', ' | ', $dictionary);
    if (str_starts_with($dictionary, '| ')) {
        $dictionary = substr($dictionary, 2);
    }

    $choices      = explode('|', $dictionary);
    $linstChoices = [];
    foreach ($choices as $choice) {
        $matches = [];
            if (preg_match("/^(\s)*([[:alnum:]]+)(\s)*,(.*)$/", $choice, $matches) !== 1) {
            throw new \DomainException("Could not parse radio option: '$choice'");

        }
        // $backend        = $matches[2] . '_'
        //        . preg_replace("/\s+/", "_", trim($matches[4]));
        $backend = $matches[2];
        $linstFormat    = "'$backend'=>'" . trim($matches[4]) . '\'';
        $linstChoices[] = $linstFormat;

    }
    return join('{-}', $linstChoices);
}

/**
 * Prints usage instruction to stderr
 *
 * @return void
 */
function showHelp()
{
    global $argv;
    fprintf(
        STDERR,
        "Usage: $argv[0] [--file=filename | --redcap-api=endpoint --redcap-token=token] --output-dir=instrumentdirectory [--trim-formname]\n"
    );
}

/**
 * Write the files to the filesystem after having parsed them.
 *
 * @param string     $outputDir   The directory to write the files
 * @param string[][] $instruments An array of fields for each instrument
 *
 * @return void
 */
function outputFiles(string $outputDir, array $instruments, array $testnameMap)
{
    fwrite(STDERR, "\n-- Writing LINST/META files\n\n");
    $db = NDB_Factory::singleton()->database();
    foreach ($instruments as $instname => $instrument) {
        fwrite(STDERR, " -> writing '$instname'\n");
        //
        $fp = fopen("$outputDir/$instname.linst", "w");
        fwrite($fp, "{-@-}testname{@}$instname\n");
        fwrite($fp, "table{@}$instname\n");
        // if no REDCap connection possible/configured
        if (isset($testnameMap[$instname])) {
            fwrite($fp, "title{@}" . $testnameMap[$instname] . "\n");
        } else {
            // try getting from LORIS db
            $instFullname = $db->pselectOne(
                "SELECT Full_name FROM test_names WHERE Test_name = :inst",
                [
                    'inst' => $instname
                ]
            );
            // title or error message
            if (!is_null($instFullname)) {
                fwrite($fp, "title{@}" . $instFullname . "\n");
            } else {
                fwrite(STDERR, "Could not get title for '$instname' for LINST file.\n");
            }
        }

        // Standard LORIS metadata fields that the instrument builder adds
        // and LINST class automatically adds to instruments.
        fwrite($fp, "date{@}Date_taken{@}Date of Administration{@}{@}\n");
        fwrite($fp, "static{@}Candidate_Age{@}Candidate Age (Years)\n");
        fwrite($fp, "static{@}Window_Difference{@}Window Difference (+/- Days)\n");
        fwrite($fp, "select{@}Examiner{@}Examiner{@}NULL=>''\n");

        foreach ($instrument as $field) {

            // avoid 'timestamp_start', changed in 'static' instead of 'text'
            if (str_contains($field, "{@}timestamp_start{@}")) {

                // transform timestamp start to static
                fwrite($fp, "static{@}timestamp_start{@}Start time (server)\n");

                // add 'timestamp_stop' and 'Duration' fields after 'timestamp_start'
                fwrite($fp, "static{@}timestamp_stop{@}Stop time (server)\n");
                fwrite($fp, "static{@}Duration{@}Duration (server) (in seconds)\n");

            } else {

                // write field line
                fwrite($fp, "$field\n");

            }
        }
        fclose($fp);

        // META file
        $fpMeta = fopen("$outputDir/$instname.meta", "w");
        fwrite($fpMeta, "testname{@}$instname\n");
        fwrite($fpMeta, "table{@}$instname\n");
        fwrite($fpMeta, "jsondata{@}true\n");
        fwrite($fpMeta, "norules{@}true");
        fclose($fpMeta);
    }
}

/**
 * Gets a file pointer pointing to the CSV stream.
 */
function getDictionaryCSVStream($redcapAPIURL, $redcapAPIToken, $inputFile) {
    // If a local input file was specified just open it
    if (!empty($inputFile)) {
        $fp = fopen($inputFile, "r");
        if ($fp === false) {
            fprintf(STDERR, "Could not open file $inputFile\n");
            exit(1);
        }
        return $fp;
    }
    $output = REDCapAPIRequest($redcapAPIURL, $redcapAPIToken, 'metadata', 'csv');
    // fgetcsv expects a filepointer, so convert the string to a data stream.
    $fp = fopen("data://text/plain;base64," . base64_encode($output), "r");
    if ($fp === false) {
        fprintf(STDERR, "Could not open stream\n");
        exit(1);
    }
    return $fp;
}

function getTestNameMapping($redcapAPIURL, $redcapAPIToken) {
    if (empty($redcapAPIURL) && empty($redcapAPIToken)) {
        return [];
    }
    $result = REDCapAPIRequest($redcapAPIURL, $redcapAPIToken, 'instrument', 'json');
    $converted = [];
    foreach (json_decode($result) as $row) {
        $converted[$row->instrument_name] = $row->instrument_label;
    }
    return $converted;
}

function REDCapAPIRequest($redcapAPIURL, $redcapAPIToken, $content, $format) {
    $data = array(
        'token' => $redcapAPIToken,
        'content' => $content,
        'format' => $format,
        'returnFormat' => 'json',
    );
    $ch = curl_init();
    curl_setopt($ch, CURLOPT_URL, $redcapAPIURL);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
    curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, false);
    curl_setopt($ch, CURLOPT_VERBOSE, 0);
    curl_setopt($ch, CURLOPT_FOLLOWLOCATION, true);
    curl_setopt($ch, CURLOPT_AUTOREFERER, true);
    curl_setopt($ch, CURLOPT_MAXREDIRS, 10);
    curl_setopt($ch, CURLOPT_CUSTOMREQUEST, 'POST');
    curl_setopt($ch, CURLOPT_FRESH_CONNECT, 1);
    curl_setopt($ch, CURLOPT_POSTFIELDS, http_build_query($data, '', '&'));
    $output = curl_exec($ch);
    curl_close($ch);
    return $output;
}
