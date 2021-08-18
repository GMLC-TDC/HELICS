function v = HELICS_FLAG_SINGLE_THREAD_FEDERATE()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 39);
  end
  v = vInitialized;
end
