function v = HELICS_FLAG_PROFILING()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 53);
  end
  v = vInitialized;
end
