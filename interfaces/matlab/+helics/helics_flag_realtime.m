function v = helics_flag_realtime()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1936535399);
  end
  v = vInitialized;
end
