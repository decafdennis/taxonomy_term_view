<?php
// Developed by Dennis Stevense, naquah.net

/**
 * Implementation of hook_page().
 */
function taxonomy_term_view_page($str_tids = '', $depth = 0, $op = 'page') {
  $terms = taxonomy_terms_parse_string($str_tids);
  if (count($terms['tids']) != 1 || $depth != 0 || $op != 'page') {
    module_load_include('inc', 'taxonomy', 'taxonomy.pages');
    return taxonomy_term_page($str_tids, $depth, $op);
  }
  $term = taxonomy_get_term(array_pop($terms['tids']));
  
  $result = db_query("SELECT * FROM {taxonomy_term_view} WHERE tid = %d", $term->tid);
  if ($record = db_fetch_array($result)) {
    $view_name = $record['view_name'];
    $display_id = $record['display_id'];
  }
  else {
    $default = variable_get('taxonomy_term_view_vocabulary_' . $term->vid, '_none');
    if ($default == '_none') {
      $view_name = '_none';
    }
    else {
      $pos = strpos($default, '__');
      $view_name = substr($default, 0, $pos);
      $display_id = substr($default, $pos + 2);
    }
  }
  
  if ($view_name == '_none') {
    module_load_include('inc', 'taxonomy', 'taxonomy.pages');
    return taxonomy_term_page($term->tid);
  }
  else {
    $view = views_get_view($view_name);
    if (!$view || !$view->set_display($display_id)) {
      return drupal_not_found();
    }
    // if (!$view->access()) {
    //   return drupal_access_denied();
    // }
    
    return $view->execute_display(NULL, array($term->tid));
  }
}
