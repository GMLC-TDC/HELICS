function v = helics_delay_filter()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 26);
  end
  v = vInitialized;
end
