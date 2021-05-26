function v = helics_handle_option_ignore_unit_mismatch()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 101);
  end
  v = vInitialized;
end
