function v = HELICS_FLAG_RESTRICTIVE_TIME_POLICY()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 35);
  end
  v = vInitialized;
end
