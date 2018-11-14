function v = helics_flag_realtime()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1398230897);
  end
  v = vInitialized;
end
