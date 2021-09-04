function v = helics_flag_ignore()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 48);
  end
  v = vInitialized;
end
