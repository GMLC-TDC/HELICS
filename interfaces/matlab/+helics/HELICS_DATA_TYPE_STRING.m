function v = HELICS_DATA_TYPE_STRING()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1432107659);
  end
  v = vInitialized;
end
