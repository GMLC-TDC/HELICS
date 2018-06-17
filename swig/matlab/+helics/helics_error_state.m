function v = helics_error_state()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 795176324);
  end
  v = vInitialized;
end
