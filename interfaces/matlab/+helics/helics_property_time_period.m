function v = helics_property_time_period()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1464812682);
  end
  v = vInitialized;
end
