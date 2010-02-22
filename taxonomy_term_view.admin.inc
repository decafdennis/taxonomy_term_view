<?php
// Developed by Dennis Stevense, naquah.net

function taxonomy_term_view_settings() {
  $options = taxonomy_term_view_options(FALSE, TRUE);
  unset($options['_none']);
  
  $form['taxonomy_term_view_options'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Enabled views and displays'),
    '#default_value' => variable_get('taxonomy_term_view_options', array_keys($options)),
    '#options' => $options,
    '#description' => t('Limit the views and displays that can be selected. Note that this will not change existing term view settings.'),
  );
  
  $form['help']['taxonomy_term_view_help'] = array(
    '#type' => 'textarea',
    '#title' => t('Custom help text'),
    '#default_value' => variable_get('taxonomy_term_view_help', ''),
    '#rows' => 10,
    '#description' => t('Any custom help text that needs to be displayed in the term view field group.'),
  );
  $form['help']['taxonomy_term_view_help_format'] = filter_form(variable_get('taxonomy_term_view_help_format', FILTER_FORMAT_DEFAULT));
  
  return system_settings_form($form);
}

function taxonomy_term_view_vocabulary_form_alter(&$form, $form_state) {
  $vid = $form['vid']['#value'];
  $default_value = variable_get('taxonomy_term_view_vocabulary_' . $vid, '_none');
    
  $form['taxonomy_term_view'] = array(
    '#type' => 'fieldset',
    '#title' => t('Term view'),
    '#description' => t('Select the default view and display to use for listing the terms in this vocabulary.'),
    '#collapsible' => TRUE,
    '#weight' => 8,
    '#tree' => TRUE,
  );
  $form['taxonomy_term_view']['view'] = array(
    '#type' => 'select',
    '#title' => t('Default view and display'),
    '#options' => taxonomy_term_view_options(FALSE),
    '#default_value' => $default_value,
    '#description' => t('Only page displays (and thus views that have at least one page display) can be selected.'),
  );
  
  $form['submit']['#weight'] = 49;
  $form['delete']['#weight'] = 50;
  
  $form['#submit'][] = 'taxonomy_term_view_vocabulary_form_submit';
}

function taxonomy_term_view_vocabulary_form_submit($form, &$form_state) {
  $vid = $form_state['values']['vid'];
  $value = $form_state['values']['taxonomy_term_view']['view'];
  
  variable_set('taxonomy_term_view_vocabulary_' . $vid, $value);
}

function taxonomy_term_view_term_form_alter(&$form, $form_state) {
  $term = $form['#term'];
  $tid = $term['tid'];
  $vid = $term['vid'];

  $result = db_query("SELECT * FROM {taxonomy_term_view} WHERE tid = %d", $tid);
  if ($record = db_fetch_array($result)) {
    if ($record['view_name'] == '_none') {
      $default_value = '_none';
    }
    else {
      $default_value = $record['view_name'] . '__' . $record['display_id'];
    }
  }
  else {
    $default_value = '_default';
  }
  
  $form['taxonomy_term_view'] = array(
    '#type' => 'fieldset',
    '#title' => t('Term view'),
    '#description' => t('Select the view and display to use for the listing of this term.'),
    '#collapsible' => TRUE,
    '#weight' => 8,
    '#tree' => TRUE,
  );
  $form['taxonomy_term_view']['view'] = array(
    '#type' => 'select',
    '#title' => t('View and display'),
    '#options' => taxonomy_term_view_options(),
    '#default_value' => $default_value,
    '#description' => t('Only page displays (and thus views that have at least one page display) can be selected.'),
  );
  
  $vocabulary_value = variable_get('taxonomy_term_view_vocabulary_' . $vid, '_none');
  $vocabulary_value_text = $form['taxonomy_term_view']['view']['#options'][$vocabulary_value];
  $form['taxonomy_term_view']['#description'] .= ' ' . t('The vocabulary default is <em>@value</em>.', array('@value' => $vocabulary_value_text));
  
  // Show custom help text
  if ($help = variable_get('taxonomy_term_view_help', '')) {
    $help_format = variable_get('taxonomy_term_view_help_format', FILTER_FORMAT_DEFAULT);
    $form['taxonomy_term_view']['#description'] .= '<div class="custom">' . check_markup($help, $help_format, FALSE) . '</div>';
  }
  
  $form['submit']['#weight'] = 49;
  $form['delete']['#weight'] = 50;
  
  $form['#submit'][] = 'taxonomy_term_view_term_form_submit';
}

function taxonomy_term_view_term_form_submit($form, &$form_state) {
  $tid = $form_state['values']['tid'];
  $value = $form_state['values']['taxonomy_term_view']['view'];
  
  db_query("DELETE FROM {taxonomy_term_view} WHERE tid = %d", $tid);
  if ($value == '_none') {
    db_query("INSERT INTO {taxonomy_term_view} (tid, view_name, display_id) VALUES (%d, '%s', '%s')", $tid, '_none', '');
  }
  else if ($value != '_default') {
    $pos = strpos($value, '__');
    $view_name = substr($value, 0, $pos);
    $display_id = substr($value, $pos + 2);
    
    db_query("INSERT INTO {taxonomy_term_view} (tid, view_name, display_id) VALUES (%d, '%s', '%s')", $tid, $view_name, $display_id);
  }
}

function taxonomy_term_view_options($include_default = TRUE, $include_all = FALSE) {
  $options = array();
  if ($include_default) {
    $options['_default'] = '<' . t('default') . '>';
  }
  $options['_none'] = '<' . t('none') . '>';
  
  $views = views_get_all_views();
  foreach ($views as $view) {
    if (!empty($view->disabled) || empty($view->display)) {
      continue;
    }
    
    $name = $view->name;
    
    foreach (array_keys($view->display) as $id) {
      if ($view->display[$id]->display_plugin != 'page') {
        continue;
      }
      
      $options[$name . '__' . $id] = $name . ': ' . $view->display[$id]->display_title;
    }
  }
  
  // Intersect with the set of enabled options
  if (!$include_all) {
    $enabled_options = array_filter(variable_get('taxonomy_term_view_options', array_keys($options)));
    $enabled_options[] = '_none';
    $enabled_options[] = '_default';

    $options = array_intersect_key($options, array_flip($enabled_options));
  }
  
  return $options;
}
