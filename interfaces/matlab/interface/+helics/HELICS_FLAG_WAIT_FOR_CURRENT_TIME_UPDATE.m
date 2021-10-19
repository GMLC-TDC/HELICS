function v = HELICS_FLAG_WAIT_FOR_CURRENT_TIME_UPDATE()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 34);
  end
  v = vInitialized;
end
