function v = helics_delay_filter()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183062);
  end
  v = vInitialized;
end
