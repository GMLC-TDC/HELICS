function v = helics_flag_observer()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 25);
  end
  v = vInitialized;
end
