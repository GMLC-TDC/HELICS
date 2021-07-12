function v = HELICS_CORE_TYPE_HTTP()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 11);
  end
  v = vInitialized;
end
