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
  $tid = array_pop($terms['tids']);
  
  $active_view = taxonomy_term_view_view($tid);
  $view_name = $active_view['view_name'];
  $display_id = $active_view['display_id'];

  if ($view_name == '_none') {
    module_load_include('inc', 'taxonomy', 'taxonomy.pages');
    return taxonomy_term_page($tid);
  }
  else {
    $view = views_get_view($view_name);
    if (!$view || !$view->set_display($display_id)) {
      return drupal_not_found();
    }
    // if (!$view->access()) {
    //   return drupal_access_denied();
    // }
    
    return $view->execute_display(NULL, array($tid));
  }
}
