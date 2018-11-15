function v = helics_flag_realtime()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183053);
  end
  v = vInitialized;
end
