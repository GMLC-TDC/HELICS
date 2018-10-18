function v = HELICS_CORE_TYPE_ZMQ_TEST()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1432107658);
  end
  v = vInitialized;
end
