function v = HELICS_CORE_TYPE_ZMQ_TEST()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1936535425);
  end
  v = vInitialized;
end
