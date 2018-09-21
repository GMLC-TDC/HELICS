function v = helics_registration_failure()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1432107588);
  end
  v = vInitialized;
end
