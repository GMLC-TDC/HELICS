function v = HELICS_FLAG_STRICT_CONFIG_CHECKING()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 42);
  end
  v = vInitialized;
end
