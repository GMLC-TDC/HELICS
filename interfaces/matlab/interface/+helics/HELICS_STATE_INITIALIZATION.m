function v = HELICS_STATE_INITIALIZATION()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 135);
  end
  v = vInitialized;
end
