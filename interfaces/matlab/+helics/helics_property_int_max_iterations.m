function v = helics_property_int_max_iterations()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183099);
  end
  v = vInitialized;
end
