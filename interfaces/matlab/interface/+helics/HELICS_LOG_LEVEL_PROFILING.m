function v = HELICS_LOG_LEVEL_PROFILING()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 54);
  end
  v = vInitialized;
end
