function v = helics_flag_debugging()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 50);
  end
  v = vInitialized;
end
