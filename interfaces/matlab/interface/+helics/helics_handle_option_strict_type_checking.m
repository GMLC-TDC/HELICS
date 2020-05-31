function v = helics_handle_option_strict_type_checking()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 94);
  end
  v = vInitialized;
end
