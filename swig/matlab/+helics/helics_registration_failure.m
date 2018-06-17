function v = helics_registration_failure()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 795176304);
  end
  v = vInitialized;
end
