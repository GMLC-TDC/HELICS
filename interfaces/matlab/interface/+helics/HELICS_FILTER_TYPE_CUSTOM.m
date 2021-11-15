function v = HELICS_FILTER_TYPE_CUSTOM()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 118);
  end
  v = vInitialized;
end
