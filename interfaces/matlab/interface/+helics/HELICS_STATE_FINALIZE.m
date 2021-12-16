function v = HELICS_STATE_FINALIZE()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 142);
  end
  v = vInitialized;
end
