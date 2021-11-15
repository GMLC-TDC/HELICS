function v = HELICS_FLAG_PROFILING_MARKER()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 55);
  end
  v = vInitialized;
end
