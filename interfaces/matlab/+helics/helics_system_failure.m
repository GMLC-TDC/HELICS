function v = helics_system_failure()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1398230858);
  end
  v = vInitialized;
end
