function v = helics_handle_option_clear_priority_list()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 101);
  end
  v = vInitialized;
end
