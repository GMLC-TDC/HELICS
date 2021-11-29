function v = HELICS_FLAG_RESTRICTIVE_TIME_POLICY()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 37);
  end
  v = vInitialized;
end
