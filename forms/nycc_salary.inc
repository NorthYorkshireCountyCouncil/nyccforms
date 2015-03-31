<?php

function salary_calc_form($form, &$form_state) {
  
//Create the redundancy calc service ref
$service = wsclient_service_load('salary_calculator');

//Make the call
$response = $service->invoke('GetBandOptions', array());

$bandOptions = $response->GetBandOptionsResult->string;

$bandOptions = array_combine($bandOptions, $bandOptions);

$form['top_text'] = array(
  '#markup' => '<p>Salary calculator 2015/16 includes 2.2% inflation & 2 days CUP.</p>'
);

   // If the form has been submitted, output something related to the submission
  if ($form_state['submitted'] && $form_state['values']['submit_button'] != 'Re-calculate') 
  {
    $form['submission'] = array(
      '#markup' => salary_calc_results_output($form_state));

	$form['#attributes'] = array('class' => array('form-results'));


	$form['submit_button'] = array(
	'#type' => 'submit',
	'#value' => t('Re-calculate'),
  	);

  }
  else {

	   $form['band'] = array(
	       '#type' => 'select',
	       '#title' => t('Select band'),
	       '#options' => $bandOptions,
	       '#required' => TRUE, 
		    '#ajax' => array(
		      'callback' => 'ajax_spinal_callback',
		      'wrapper' => 'spinal-div',
		      'method' => 'replace',
		      'effect' => 'fade',
	    ),	       
		   );


		 $form['spinal'] = array(
		    '#title' => t("Spinal point"),
		    // The prefix/suffix provide the div that we're replacing, named by
		    // #ajax['wrapper'] above.
		    '#prefix' => '<div id="spinal-div">',
		    '#suffix' => '</div>',
		    '#type' => 'select',
		    '#description' => t('Select the appropriate spinal point'),
		  );

		$form['fte'] = array(
		    '#type' => 'textfield', 
		    '#title' => 'FTE',
		    '#size' => 4,
		    '#maxlength' => 4,
		    '#default_value' => '1.00',
		    '#required' => TRUE, 
		    '#description' => t('Please enter a value between 0 and 1 eg. 0.61'),
	  	);


		$form['submit_button'] = array(
		'#type' => 'submit',
		'#value' => t('Calculate salary'),
	  	);
	}

  return $form;
}

/**
 * Callback element needs only select the portion of the form to be updated.
 * Since #ajax['callback'] return can be HTML or a renderable array (or an
 * array of commands), we can just return a piece of the form.
 */
function ajax_spinal_callback($form, $form_state) {

	//Create the redundancy calc service ref
	$service = wsclient_service_load('salary_calculator');

	$SpinalPointsReq = array('parameters' => array('band' => $form_state['values']['band']));

	//Make the call
	$response = $service->invoke('GetSpinalPoints', $SpinalPointsReq);

	$spinalOptions = $response->GetSpinalPointsResult->string;

	//If only 1 result, comes back as a string, needs to be an array
	if(gettype($spinalOptions) == "string")
	{
		$spinalOptions = array($spinalOptions);
	}

	$spinalOptions = array_combine($spinalOptions, $spinalOptions);


	$form['spinal'] = array(
	    '#title' => t("Spinal point"),
	    '#name' => 'spinal',
	    // The prefix/suffix provide the div that we're replacing, named by
	    // #ajax['wrapper'] above.
	    '#prefix' => '<div id="spinal-div">',
	    '#suffix' => '</div>',
	    '#type' => 'select',
	    '#description' => t('Select the appropriate spinal point'),
	    '#options' => $spinalOptions,

	);

  return $form['spinal'];
}



function salary_calc_form_validate($form, &$form_state) {

	if ($form_state['values']['submit_button'] != 'Re-calculate') 
	{
		if (($form_state['values']['fte'] < 0) || ($form_state['values']['fte'] > 1) || (!is_numeric($form_state['values']['fte'])) ){
			form_set_error('fte', t('FTE must be between 0 and 1'));
		}	
	}

}


function salary_calc_form_submit($form, &$form_state) {
	
	if ($form_state['values']['submit_button'] != 'Re-calculate') 
	{
		//Create the redundancy calc service ref
		$service = wsclient_service_load('salary_calculator');


		$SalarySummaryReq = array('parameters' => array('fte' => $form_state['values']['fte'], 'spinalPoint' => $form_state['values']['spinal']));

		//Make the call
		$response = $service->invoke('GetSalarySummary', $SalarySummaryReq);

		$summary = $response->GetSalarySummaryResult->string;

		$form_state['result']['salary'] = number_format($summary[0], 2);
		$form_state['result']['salnipension'] = number_format($summary[1], 2);
		$form_state['result']['employeehourly'] = number_format($summary[2], 2);
		$form_state['result']['employerhourly'] = number_format($summary[3], 2);
		$form_state['result']['detail'] = "Results for {$form_state['values']['band']}, spinal point {$form_state['values']['spinal']} and {$form_state['values']['fte']} FTE.";

	}

	$form_state['rebuild'] = TRUE;
}


function salary_calc_results_output(&$form_state) {

	$markup = "
	<p>{$form_state['result']['detail']}</p>
	<h3>Salary:</h3>
	<div>£{$form_state['result']['salary']}</div>
	<h3>Salary + Employer National Insurance + Pension:</h3>
	<div>£{$form_state['result']['salnipension']}</div>
	<h3>Hourly rate salary only:</h3>
	<div>£{$form_state['result']['employeehourly']}</div>
	<h3>Hourly rate employer (SAL + NI + PEN):</h3>
	<div>£{$form_state['result']['employerhourly']}</div>
	<br />		
	";

	return $markup;
}