function v = HELICS_FLAG_LOCAL_PROFILING_CAPTURE()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 46);
  end
  v = vInitialized;
end
