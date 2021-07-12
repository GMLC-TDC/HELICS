function v = HELICS_CORE_TYPE_ZMQ_SS()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 8);
  end
  v = vInitialized;
end
