function v = helics_error()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1432107596);
  end
  v = vInitialized;
end
