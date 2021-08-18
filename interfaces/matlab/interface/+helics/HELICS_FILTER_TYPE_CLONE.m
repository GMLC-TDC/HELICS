function v = HELICS_FILTER_TYPE_CLONE()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 122);
  end
  v = vInitialized;
end
