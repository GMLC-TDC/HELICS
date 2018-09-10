function v = helics_other_error()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1936535365);
  end
  v = vInitialized;
end
