function v = helics_time_property_period()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1128095536);
  end
  v = vInitialized;
end
