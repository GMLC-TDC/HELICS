function v = HELICS_CORE_TYPE_ZMQ()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 795176336);
  end
  v = vInitialized;
end
