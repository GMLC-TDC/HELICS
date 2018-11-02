function v = HELICS_CORE_TYPE_UDP()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1593856925);
  end
  v = vInitialized;
end
