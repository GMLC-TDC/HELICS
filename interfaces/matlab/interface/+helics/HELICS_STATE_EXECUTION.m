function v = HELICS_STATE_EXECUTION()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 138);
  end
  v = vInitialized;
end
