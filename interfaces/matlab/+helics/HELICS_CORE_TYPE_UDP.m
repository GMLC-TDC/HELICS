function v = HELICS_CORE_TYPE_UDP()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 70);
  end
  v = vInitialized;
end
