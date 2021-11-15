function v = HELICS_FLAG_SINGLE_THREAD_FEDERATE()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 40);
  end
  v = vInitialized;
end
