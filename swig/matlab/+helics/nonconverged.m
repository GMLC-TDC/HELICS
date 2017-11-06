function v = nonconverged()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 4);
  end
  v = vInitialized;
end
