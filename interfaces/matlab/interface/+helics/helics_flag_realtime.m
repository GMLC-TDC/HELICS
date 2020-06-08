function v = helics_flag_realtime()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 37);
  end
  v = vInitialized;
end
