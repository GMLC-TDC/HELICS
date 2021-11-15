function v = HELICS_PROPERTY_INT_MAX_ITERATIONS()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 91);
  end
  v = vInitialized;
end
